/*****************************************************************************
 * Copyright (c) 2014-2024 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <cstdint>
#include <openrct2/world/Footpath.h>

namespace OpenRCT2::Ui
{
    void ProvisionalElementsRemove();
    void ProvisionalElementsRestore();

    /**
     * Called after building something else, which might affect the ability to build paths there.
     */
    void FootpathRecheckProvisional();
    /**
     * Permanently remove provisional footpaths - e.g. after successful placement, or when cancelling placement.
     */
    void FootpathRemoveProvisional();
    void FootpathUpdateProvisional();
    /**
     * Used when performing actions on footpaths where provisional elements may interfere, and where the provisional
     * elements must be restored later on (which can be done by calling `WindowFootpathRestoreProvisional()`.
     */
    void FootpathRemoveProvisionalTemporarily();
    /**
     * Restore provisional elements that have been removed using `WindowFootpathRemoveProvisionalTemporarily()`.
     */
    void FootpathRestoreProvisional();

    money64 FootpathProvisionalSet(
        ObjectEntryIndex type, ObjectEntryIndex railingsType, const CoordsXYZ& footpathLoc, int32_t slope,
        PathConstructFlags constructFlags);

    enum class ProvisionalPathFlag : uint8_t
    {
        showArrow = 0,
        /**
         * Set when any provisional path is present.
         */
        placed = 1,
        forceRecheck = 2,
    };

    struct ProvisionalFootpath
    {
        ObjectEntryIndex type;
        CoordsXYZ position;
        uint8_t slope;
        uint8_t flags;
        ObjectEntryIndex surfaceIndex;
        ObjectEntryIndex railingsIndex;
        PathConstructFlags constructFlags;

        bool hasFlag(ProvisionalPathFlag flag)
        {
            return HasFlag(flags, flag);
        }

        void setFlag(ProvisionalPathFlag flag)
        {
            flags |= EnumToFlag(flag);
        }

        void clearFlag(ProvisionalPathFlag flag)
        {
            flags &= ~EnumToFlag(flag);
        }

        void flipFlag(ProvisionalPathFlag flag)
        {
            flags ^= EnumToFlag(flag);
        }
    };

    extern ProvisionalFootpath _provisionalFootpath;

} // namespace OpenRCT2::Ui
