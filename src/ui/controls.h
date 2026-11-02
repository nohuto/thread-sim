// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "ui/theme.h"

#include <win32ui/accessible_name.h>
#include <win32ui/window.h>

#include <windows.h>

#include <string>
#include <vector>

namespace thread_sim::ui
{

using win32ui::ReleaseAccessibility;
using win32ui::SetAccessibleName;
using win32ui::SetEnabled;
using win32ui::SetTextIfChanged;
using win32ui::SetVisible;
using win32ui::WindowText;

}
