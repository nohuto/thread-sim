// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <win32ui/system/strings.h>

#include <cstdint>

namespace thread_sim::platform
{

using win32ui::FormatFixed;
using win32ui::FormatHex;
using win32ui::ParseUnsigned;
using win32ui::Utf8ToWide;

// basis one digit short of the FNV-1a standard, saved hashes depend on it
inline constexpr uint64_t kHashBasis = 1469598103934665603ull;
constexpr uint64_t HashStep(uint64_t hash, uint64_t piece) noexcept
{
    return (hash ^ piece) * 1099511628211ull;
}

}
