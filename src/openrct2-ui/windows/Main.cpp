/*****************************************************************************
 * Copyright (c) 2014-2024 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../UiStringIds.h"

#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/config/Config.h>
#include <openrct2/world/Footpath.h>

namespace OpenRCT2::Ui::Windows
{
    // clang-format off
    static Widget _mainWidgets[] = {
        MakeWidget({0, 0}, {0, 0}, WindowWidgetType::Viewport, WindowColour::Primary),
        kWidgetsEnd,
    };
    // clang-format on

    class MainWindow final : public Window
    {
    public:
        void OnOpen() override
        {
            _mainWidgets[0].right = width;
            _mainWidgets[0].bottom = height;
            widgets = _mainWidgets;

            ViewportCreate(this, windowPos, width, height, Focus(CoordsXYZ(0x0FFF, 0x0FFF, 0)));
            if (viewport != nullptr)
            {
                SetViewportFlags(gScreenFlags & SCREEN_FLAGS_TITLE_DEMO);
                viewport->rotation = 0;
            }
            gShowGridLinesRefCount = 0;
            gShowLandRightsRefCount = 0;
            gShowConstructionRightsRefCount = 0;
            WindowFootpathResetSelectedPath();
        }

        void OnDraw(DrawPixelInfo& dpi) override
        {
            ViewportRender(dpi, viewport);
        }

    private:
        void SetViewportFlags(bool isTitleWindow)
        {
            viewport->flags |= VIEWPORT_FLAG_SOUND_ON;
            if (isTitleWindow)
            {
                return;
            }

            if (Config::Get().general.InvisibleRides)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_RIDES;
                viewport->flags |= VIEWPORT_FLAG_HIDE_RIDES;
            }
            if (Config::Get().general.InvisibleVehicles)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_VEHICLES;
                viewport->flags |= VIEWPORT_FLAG_HIDE_VEHICLES;
            }
            if (Config::Get().general.InvisibleTrees)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_VEGETATION;
                viewport->flags |= VIEWPORT_FLAG_HIDE_VEGETATION;
            }
            if (Config::Get().general.InvisibleScenery)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_SCENERY;
                viewport->flags |= VIEWPORT_FLAG_HIDE_SCENERY;
            }
            if (Config::Get().general.InvisiblePaths)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_PATHS;
                viewport->flags |= VIEWPORT_FLAG_HIDE_PATHS;
            }
            if (Config::Get().general.InvisibleSupports)
            {
                viewport->flags |= VIEWPORT_FLAG_INVISIBLE_SUPPORTS;
                viewport->flags |= VIEWPORT_FLAG_HIDE_SUPPORTS;
            }
        }
    };

    /**
     * Creates the main window that holds the main viewport.
     *  rct2: 0x0066B3E8
     */
    WindowBase* MainOpen()
    {
        return WindowCreate<MainWindow>(
            WindowClass::MainWindow, { 0, 0 }, ContextGetWidth(), ContextGetHeight(), WF_STICK_TO_BACK);
    }
} // namespace OpenRCT2::Ui::Windows
