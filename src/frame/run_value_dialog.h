// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "engine/config.h"
#include "engine/validation.h"

#include "platform/topology.h"

#include <windows.h>

namespace thread_sim::frame
{

bool ShowRunValueDialog(HWND owner, engine::RunConfig* config, engine::Field field);

}
