// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "engine/config.h"
#include "engine/snapshot.h"

#include <windows.h>

#include <string>
#include <vector>

namespace thread_sim::frame
{

inline constexpr DWORD kPriorityClasses[] = {IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS};
std::wstring PriorityClassName(DWORD priority_class);

std::wstring ThreadStateText(const engine::ThreadSnapshot& thread, uint32_t windows_build);
std::wstring DescribeActivity(const engine::ActivityConfig& activity, bool uses_shared_mutex);
std::wstring DescribeDuration(uint32_t seconds);
inline std::wstring DescribeDuration(std::optional<uint32_t> seconds)
{
    return seconds ? DescribeDuration(*seconds) : L"Unlimited";
}

std::wstring DescribeInterval(uint32_t milliseconds);
std::wstring DescribeCpuSetIds(const std::vector<uint32_t>& ids);

}
