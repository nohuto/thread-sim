// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "platform/text.h"

#include <cstdio>

namespace thread_sim::platform
{

std::wstring FormatUnsigned(uint64_t value)
{
    wchar_t buffer[32] = {};
    swprintf_s(buffer, L"%llu", static_cast<unsigned long long>(value));
    return buffer;
}

}
