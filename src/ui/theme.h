// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <win32ui/dialog/dialog_layout.h>
#include <win32ui/status_bar/status_bar.h>
#include <win32ui/theme/theme.h>
#include <win32ui/toolbar/toolbar.h>

#include <windows.h>

namespace thread_sim::ui
{

using win32ui::SetTextRole;
using win32ui::TextRole;

win32ui::ToolbarHooks ToolbarHooks();
win32ui::StatusBarHooks StatusBarHooks();

}
