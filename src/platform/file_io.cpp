// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "platform/file_io.h"

#include <win32ui/system/file_io.h>
#include <win32ui/system/paths.h>
#include <win32ui/system/strings.h>

#include <shlobj.h>

namespace thread_sim::platform
{

Result<std::wstring> ReadUtf8File(const std::wstring& path, uint64_t max_bytes)
{
    std::wstring text;
    switch (const DWORD code = win32ui::ReadTextFile(path.c_str(), &text, max_bytes, false))
    {
    case ERROR_SUCCESS:
        return text;
    case ERROR_FILE_TOO_LARGE:
        return FormatError(L"Read file", L"The file is larger than the supported limit.").WithContext(path);
    case ERROR_INVALID_DATA:
        return FormatError(L"Decode UTF-8", L"The file is not valid UTF-8 text.").WithContext(path);
    default:
        return Win32Error(code, L"Read file").WithContext(path);
    }
}

Status WriteFileAtomic(const std::wstring& path, std::string_view bytes)
{
    const DWORD code = win32ui::WriteFileAtomic(path.c_str(), bytes);
    return code == ERROR_SUCCESS ? Status::Ok() : Status(Win32Error(code, L"Write file").WithContext(path));
}

Status WriteUtf8FileAtomic(const std::wstring& path, std::wstring_view text)
{
    const std::optional<std::string> bytes = win32ui::WideToUtf8(text);
    if (!bytes)
    {
        return FormatError(L"Encode UTF-8", L"The text contains invalid UTF-16.").WithContext(path);
    }
    return WriteFileAtomic(path, *bytes);
}

Status EnsureDirectory(const std::wstring& path)
{
    if (path.empty())
    {
        return ValidationError(L"Create directory", L"The directory path is empty.");
    }
    switch (const DWORD code = win32ui::EnsureDirectory(path.c_str()))
    {
    case ERROR_SUCCESS:
        return Status::Ok();
    case ERROR_ALREADY_EXISTS:
        return ValidationError(L"Create directory", L"The path exists but is not a directory.").WithContext(path);
    default:
        return Win32Error(code, L"Create directory").WithContext(path);
    }
}

std::wstring DefaultDataDirectory()
{
    const std::wstring local = win32ui::KnownFolderPath(FOLDERID_LocalAppData);
    return local.empty() ? std::wstring() : win32ui::JoinPath(local, L"thread-sim");
}

}
