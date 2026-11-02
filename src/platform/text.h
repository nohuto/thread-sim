// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <win32ui/system/strings.h>

#include <cstdint>
#include <string>

namespace thread_sim::platform
{

using win32ui::FormatFixed;
using win32ui::FormatHex;
using win32ui::ParseUnsigned;
using win32ui::Utf8ToWide;

std::wstring FormatUnsigned(uint64_t value);

}
