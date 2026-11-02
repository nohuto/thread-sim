// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "platform/error.h"

#include <win32ui/system/error_text.h>

#include <cstdio>

namespace thread_sim::platform
{

namespace
{

Error Make(ErrorDomain domain, int64_t code, std::wstring_view operation, std::wstring_view detail = {})
{
    Error error;
    error.domain = domain;
    error.code = code;
    error.operation = operation;
    error.detail = detail;
    return error;
}

std::wstring FormatNtStatus(LONG status)
{
    return win32ui::FormatModuleError(static_cast<DWORD>(status), L"ntdll.dll");
}

}

std::wstring Error::CodeText() const
{
    wchar_t buffer[64] = {};
    switch (domain)
    {
    case ErrorDomain::kWin32:
        swprintf_s(buffer, L"Win32 %lld", static_cast<long long>(code));
        break;
    case ErrorDomain::kHResult:
        swprintf_s(buffer, L"HRESULT 0x%08X", static_cast<unsigned>(code));
        break;
    case ErrorDomain::kNtStatus:
        swprintf_s(buffer, L"NTSTATUS 0x%08X", static_cast<unsigned>(code));
        break;
    case ErrorDomain::kCrt:
        swprintf_s(buffer, L"errno %lld", static_cast<long long>(code));
        break;
    default:
        break;
    }
    return buffer;
}

std::wstring Error::Describe() const
{
    std::wstring text;
    if (!context.empty())
    {
        text += context;
        text += L": ";
    }
    if (!operation.empty())
    {
        text += operation;
        text += L" failed";
    }
    std::wstring explanation = detail;
    if (explanation.empty() && domain == ErrorDomain::kWin32)
    {
        explanation = win32ui::FormatWin32Error(static_cast<DWORD>(code));
    }
    if (explanation.empty() && domain == ErrorDomain::kHResult)
    {
        explanation = win32ui::FormatWin32Error(static_cast<DWORD>(code));
    }
    if (explanation.empty() && domain == ErrorDomain::kNtStatus)
    {
        explanation = FormatNtStatus(static_cast<LONG>(code));
    }
    if (!explanation.empty())
    {
        text += operation.empty() ? L"" : L": ";
        text += explanation;
    }
    std::wstring code_text = CodeText();
    if (!code_text.empty() && !detail.empty() && (domain == ErrorDomain::kWin32 || domain == ErrorDomain::kHResult || domain == ErrorDomain::kNtStatus))
    {
        const std::wstring system = domain == ErrorDomain::kNtStatus ? FormatNtStatus(static_cast<LONG>(code)) : win32ui::FormatWin32Error(static_cast<DWORD>(code));
        if (!system.empty())
        {
            code_text += L": " + system;
        }
    }
    if (!code_text.empty())
    {
        text += L" (";
        text += code_text;
        text += L")";
    }
    return text;
}

Error& Error::WithContext(std::wstring_view text) &
{
    context = text;
    return *this;
}

Error&& Error::WithContext(std::wstring_view text) &&
{
    context = text;
    return std::move(*this);
}

Error Win32Error(DWORD code, std::wstring_view operation)
{
    return Make(ErrorDomain::kWin32, code, operation);
}

Error LastWin32Error(std::wstring_view operation)
{
    const DWORD code = GetLastError();
    return Make(ErrorDomain::kWin32, code, operation);
}

Error HResultError(HRESULT hr, std::wstring_view operation)
{
    return Make(ErrorDomain::kHResult, static_cast<int64_t>(static_cast<uint32_t>(hr)), operation);
}

Error NtStatusError(LONG status, std::wstring_view operation)
{
    return Make(ErrorDomain::kNtStatus, static_cast<int64_t>(static_cast<uint32_t>(status)), operation);
}

Error CrtError(int error_number, std::wstring_view operation)
{
    return Make(ErrorDomain::kCrt, error_number, operation);
}

Error ValidationError(std::wstring_view operation, std::wstring_view detail)
{
    return Make(ErrorDomain::kValidation, 0, operation, detail);
}

Error FormatError(std::wstring_view operation, std::wstring_view detail)
{
    return Make(ErrorDomain::kFormat, 0, operation, detail);
}

Error UnsupportedError(std::wstring_view operation, std::wstring_view detail)
{
    return Make(ErrorDomain::kUnsupported, 0, operation, detail);
}

Error InternalError(std::wstring_view operation, std::wstring_view detail)
{
    return Make(ErrorDomain::kInternal, 0, operation, detail);
}

}
