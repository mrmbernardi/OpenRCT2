/*****************************************************************************
 * Copyright (c) 2014-2024 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "ProvisionalElements.h"

#include <openrct2-ui/windows/Window.h>
#include <openrct2/interface/Window.h>
#include <openrct2/interface/WindowClasses.h>
#include <openrct2/interface/Viewport.h>
#include <openrct2/network/network.h>
#include <openrct2/paint/VirtualFloor.h>
#include <openrct2/profiling/Profiling.h>
#include <openrct2/ride/RideConstruction.h>
#include <openrct2/actions/FootpathRemoveAction.h>
#include <openrct2/actions/FootpathPlaceAction.h>
#include <openrct2/world/tile_element/Slope.h>

using namespace OpenRCT2::Ui::Windows;

namespace OpenRCT2::Ui
{
    ProvisionalFootpath _provisionalFootpath;

    void ProvisionalElementsRemove()
    {
        PROFILED_FUNCTION();

        FootpathRemoveProvisionalTemporarily();

        if (WindowFindByClass(WindowClass::RideConstruction) != nullptr)
        {
            RideRemoveProvisionalTrackPiece();
            RideEntranceExitRemoveGhost();
        }
        // This is in non performant so only make network games suffer for it
        // non networked games do not need this as its to prevent desyncs.
        if ((NetworkGetMode() != NETWORK_MODE_NONE) && WindowFindByClass(WindowClass::TrackDesignPlace) != nullptr)
        {
            TrackPlaceClearProvisionalTemporarily();
        }
    }

    void ProvisionalElementsRestore()
    {
        PROFILED_FUNCTION();

        FootpathRestoreProvisional();

        if (WindowFindByClass(WindowClass::RideConstruction) != nullptr)
        {
            RideRestoreProvisionalTrackPiece();
            RideEntranceExitPlaceProvisionalGhost();
        }
        // This is in non performant so only make network games suffer for it
        // non networked games do not need this as its to prevent desyncs.
        if ((NetworkGetMode() != NETWORK_MODE_NONE) && WindowFindByClass(WindowClass::TrackDesignPlace) != nullptr)
        {
            TrackPlaceRestoreProvisional();
        }
    }

    /**
     *
     *  rct2: 0x006A77FF
     */
    void FootpathRemoveProvisional()
    {
        if (_provisionalFootpath.hasFlag(ProvisionalPathFlag::placed))
        {
            _provisionalFootpath.clearFlag(ProvisionalPathFlag::placed);

            auto action = FootpathRemoveAction(_provisionalFootpath.position);
            action.SetFlags(GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED | GAME_COMMAND_FLAG_NO_SPEND | GAME_COMMAND_FLAG_GHOST);
            GameActions::Execute(&action);
        }
    }

    /**
     *
     *  rct2: 0x006A7831
     */
    void FootpathUpdateProvisional()
    {
        if (_provisionalFootpath.hasFlag(ProvisionalPathFlag::showArrow))
        {
            _provisionalFootpath.clearFlag(ProvisionalPathFlag::showArrow);

            gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;
            MapInvalidateTileFull(gFootpathConstructFromPosition);
        }
        FootpathRemoveProvisional();
    }

    void FootpathRemoveProvisionalTemporarily()
    {
        if (_provisionalFootpath.hasFlag(ProvisionalPathFlag::placed))
        {
            FootpathRemoveProvisional();
            _provisionalFootpath.setFlag(ProvisionalPathFlag::placed);
        }
    }

    void FootpathRestoreProvisional()
    {
        if (_provisionalFootpath.hasFlag(ProvisionalPathFlag::placed))
        {
            _provisionalFootpath.clearFlag(ProvisionalPathFlag::placed);
            FootpathProvisionalSet(
                _provisionalFootpath.surfaceIndex, _provisionalFootpath.railingsIndex, _provisionalFootpath.position,
                _provisionalFootpath.slope, _provisionalFootpath.constructFlags);
        }
    }

    void FootpathRecheckProvisional()
    {
        _provisionalFootpath.setFlag(ProvisionalPathFlag::forceRecheck);
    }

        /**
     *
     *  rct2: 0x006A76FF
     */
    money64 FootpathProvisionalSet(
        ObjectEntryIndex type, ObjectEntryIndex railingsType, const CoordsXYZ& footpathLoc, int32_t slope,
        PathConstructFlags constructFlags)
    {
        money64 cost;

        FootpathRemoveProvisional();

        auto footpathPlaceAction = FootpathPlaceAction(
            footpathLoc, slope, type, railingsType, INVALID_DIRECTION, constructFlags);
        footpathPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED);
        auto res = GameActions::Execute(&footpathPlaceAction);
        cost = res.Error == GameActions::Status::Ok ? res.Cost : kMoney64Undefined;
        if (res.Error == GameActions::Status::Ok)
        {
            _provisionalFootpath.surfaceIndex = type;
            _provisionalFootpath.railingsIndex = railingsType;
            _provisionalFootpath.position = footpathLoc;
            _provisionalFootpath.slope = slope;
            _provisionalFootpath.constructFlags = constructFlags;
            _provisionalFootpath.setFlag(ProvisionalPathFlag::placed);

            if (gFootpathGroundFlags & ELEMENT_IS_UNDERGROUND)
            {
                ViewportSetVisibility(ViewportVisibility::UndergroundViewOn);
            }
            else
            {
                ViewportSetVisibility(ViewportVisibility::UndergroundViewOff);
            }
        }

        // Invalidate previous footpath piece.
        VirtualFloorInvalidate();

        if (!isToolActive(WindowClass::Scenery))
        {
            if (res.Error != GameActions::Status::Ok)
            {
                // If we can't build this, don't show a virtual floor.
                VirtualFloorSetHeight(0);
            }
            else if (
                _provisionalFootpath.slope == kTileSlopeFlat
                || _provisionalFootpath.position.z < gFootpathConstructFromPosition.z)
            {
                // Going either straight on, or down.
                VirtualFloorSetHeight(_provisionalFootpath.position.z);
            }
            else
            {
                // Going up in the world!
                VirtualFloorSetHeight(_provisionalFootpath.position.z + LAND_HEIGHT_STEP);
            }
        }

        return cost;
    }
} // namespace OpenRCT2::Ui
