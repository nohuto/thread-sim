// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "ui/theme.h"

namespace thread_sim::ui
{

win32ui::ToolbarHooks ToolbarHooks()
{
    win32ui::ToolbarHooks hooks;
    hooks.frame = win32ui::ThemeDialogHooks();
    return hooks;
}

win32ui::StatusBarHooks StatusBarHooks()
{
    win32ui::StatusBarHooks hooks;
    hooks.frame = win32ui::ThemeDialogHooks();
    return hooks;
}

}
