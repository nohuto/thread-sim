// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "platform/process_settings.h"

#include "platform/topology.h"

#include <limits>

namespace thread_sim::platform
{

Result<ProcessSettings> QueryProcessSettings()
{
    ProcessSettings settings;
    settings.priority_class = GetPriorityClass(GetCurrentProcess());
    if (!settings.priority_class)
    {
        return LastWin32Error(L"GetPriorityClass");
    }
    // current process pseudo handle reports affinity in the calling primary group
    GROUP_AFFINITY group{};
    if (!GetThreadGroupAffinity(GetCurrentThread(), &group))
    {
        return LastWin32Error(L"GetThreadGroupAffinity (process group)");
    }
    settings.group = group.Group;
    DWORD_PTR process_mask = 0, system_mask = 0;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &process_mask, &system_mask))
    {
        return LastWin32Error(L"GetProcessAffinityMask");
    }
    settings.affinity = process_mask;
    settings.system_affinity = system_mask;
    Result<std::vector<uint32_t>> ids = QueryProcessDefaultCpuSets();
    if (!ids)
    {
        return std::move(ids).error();
    }
    settings.default_cpu_set_ids = std::move(ids).value();
    return settings;
}

Status SetProcessPriorityClassValue(DWORD priority_class)
{
    if (!SetPriorityClass(GetCurrentProcess(), priority_class))
    {
        return LastWin32Error(L"SetPriorityClass");
    }
    return Status::Ok();
}

Status SetProcessAffinity(uint64_t mask)
{
    if (!mask)
    {
        return ValidationError(L"SetProcessAffinityMask", L"Select at least one logical processor.");
    }
    if (!SetProcessAffinityMask(GetCurrentProcess(), static_cast<DWORD_PTR>(mask)))
    {
        return LastWin32Error(L"SetProcessAffinityMask");
    }
    return Status::Ok();
}

Status SetProcessDefaultCpuSetIds(std::span<const uint32_t> ids)
{
    if (ids.size() > std::numeric_limits<ULONG>::max())
    {
        return ValidationError(L"SetProcessDefaultCpuSets", L"Too many CPU Set IDs.");
    }
    const std::vector<ULONG> values(ids.begin(), ids.end());
    if (!SetProcessDefaultCpuSets(GetCurrentProcess(), values.empty() ? nullptr : values.data(), static_cast<ULONG>(values.size())))
    {
        return LastWin32Error(L"SetProcessDefaultCpuSets");
    }
    return Status::Ok();
}

}
