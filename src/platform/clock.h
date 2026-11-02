// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <win32ui/windows_config.h>

#include <win32ui/system/clock.h>
#include <win32ui/system/date_time.h>

#include <windows.h>

#include <cstdint>
#include <intrin.h>

namespace thread_sim::platform
{

using win32ui::FileTimeTicks;
using win32ui::QpcClock;

// cycle to time conversion requires a time stamp counter invariant across frequency & power states
inline bool InvariantTimeStampCounter() noexcept
{
    static const bool invariant = [] {
        int registers[4]{};
        __cpuid(registers, 0x80000000);
        if (static_cast<unsigned>(registers[0]) < 0x80000007u)
        {
            return false;
        }
        __cpuid(registers, 0x80000007);
        return (registers[3] & (1 << 8)) != 0;
    }();
    return invariant;
}

}
