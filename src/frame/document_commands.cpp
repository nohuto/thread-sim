// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "frame/main_window.h"

#include "frame/window_detail.h"

#include "ui/dialog.h"

#include "engine/config_io.h"
#include "engine/results_export.h"

#include "platform/environment.h"

#include <win32ui/dialog/file_dialog.h>
#include <win32ui/system/paths.h>
#include <win32ui/system/strings.h>

namespace thread_sim::frame
{

using namespace ui;
using namespace window_detail;

std::optional<std::wstring> window_detail::FileDialog(HWND owner, bool save, const wchar_t* title, const wchar_t* filter, const std::wstring& initial_name, const std::wstring& folder)
{
    std::wstring path;
    const win32ui::FileDialogSpec spec = {.filter = filter, .title = title, .initial_name = initial_name, .folder = folder};
    const DWORD error = save ? win32ui::ChooseFileToSave(owner, spec, &path) : win32ui::ChooseFileToOpen(owner, spec, &path);
    if (error == ERROR_SUCCESS)
    {
        return path;
    }
    if (error != ERROR_CANCELLED)
    {
        // Win32FromHResult keeps the bits of HRESULTs outside FACILITY_WIN32
        ShowError(owner, title, L"The file dialog could not be shown.", (error & 0x80000000u) ? platform::HResultError(static_cast<HRESULT>(error), L"File dialog") : platform::Win32Error(error, L"File dialog"));
    }
    return std::nullopt;
}

bool MainWindow::ConfirmDiscard()
{
    if (config_ == saved_config_)
    {
        return true;
    }
    switch (AskSaveChanges(hwnd_, DocumentName()))
    {
    case SaveChoice::kSave:
        return SaveDocument(false);
    case SaveChoice::kDiscard:
        return true;
    default:
        return false;
    }
}

void MainWindow::RequestClose()
{
    if (win32ui::ModalDepth() > 0)
    {
        win32ui::CancelOpenDialogs();
        SetTimer(hwnd_, kCloseRetryTimer, 50, nullptr);
        return;
    }
    if (close_pending_)
    {
        if (!RunBusy())
        {
            SaveSettings();
            DestroyWindow(hwnd_);
        }
        return;
    }
    if (!ConfirmDiscard())
    {
        return;
    }
    if (RunBusy())
    {
        close_pending_ = true;
        controller_->RequestStopAll();
        UpdateStatus();
        SetTimer(hwnd_, kReapTimer, 20, nullptr);
        return;
    }
    SaveSettings();
    DestroyWindow(hwnd_);
}

void MainWindow::NewDocument()
{
    if (!ConfirmDiscard())
    {
        return;
    }
    config_ = engine::MakeDefaultRun();
    saved_config_ = config_;
    document_path_.clear();
    ResetThreadKeys();
    Revalidate();
    table_.SetData(&config_, nullptr);
    results_stale_ = snapshot_ != nullptr;
    table_.RefreshAll();
    table_.SelectThread(0, true);
    UpdateDetails();
    UpdateTitle();
    FollowSampleInterval();
}

void MainWindow::OpenDocument(const std::wstring& path)
{
    auto loaded = engine::LoadRunConfig(path);
    if (!loaded)
    {
        ShowError(hwnd_, L"Open config", L"The config could not be opened.", loaded.error());
        return;
    }
    config_ = loaded->config;
    saved_config_ = config_;
    document_path_ = path;
    settings_.last_directory = win32ui::ParentDirectory(path);
    ResetThreadKeys();
    Revalidate();
    results_stale_ = snapshot_ != nullptr;
    table_.SetData(&config_, nullptr);
    table_.RefreshAll();
    table_.SelectThread(0, true);
    UpdateDetails();
    UpdateTitle();
    FollowSampleInterval();
    std::wstring notes;
    for (const auto& warning : loaded->warnings)
    {
        notes += warning + L"\n";
    }
    if (loaded->saved_topology && !(*loaded->saved_topology == engine::FingerprintOf(environment_.topology)))
    {
        notes += L"The config was saved on a machine with another processor topology, check its processor numbers and CPU Set IDs.\n";
    }
    if (!notes.empty())
    {
        win32ui::MessageSpec spec;
        spec.title = L"Open config";
        spec.message = L"Opened with notes.";
        spec.details = notes;
        spec.buttons = {{IDOK, L"OK"}};
        ui::ShowMessage(hwnd_, std::move(spec));
    }
    SetStatusMessage(L"Opened " + win32ui::FileName(path));
}

bool MainWindow::SaveDocument(bool save_as)
{
    std::wstring path = document_path_;
    if (save_as || path.empty())
    {
        const std::wstring name = win32ui::SafeFileName(config_.name);
        const std::wstring suggested = (name.empty() ? std::wstring(L"thread-sim-run") : name) + L".jsonc";
        const auto chosen = FileDialog(hwnd_, true, L"Save config", L"ThreadSim config (*.jsonc)\0*.jsonc\0All files\0*.*\0", suggested, settings_.last_directory);
        if (!chosen)
        {
            return false;
        }
        path = *chosen;
    }
    const platform::Status status = engine::SaveRunConfig(path, config_, environment_.topology);
    if (!status)
    {
        ShowError(hwnd_, L"Save config", L"The config could not be saved.", status.error());
        return false;
    }
    document_path_ = path;
    saved_config_ = config_;
    settings_.last_directory = win32ui::ParentDirectory(path);
    UpdateTitle();
    SetStatusMessage(L"Saved " + win32ui::FileName(path));
    return true;
}

void MainWindow::ExportResults()
{
    if (!snapshot_ || !snapshot_->final)
    {
        return;
    }
    const std::wstring suggested = L"thread-sim-" + snapshot_->run_id + L".csv";
    const auto chosen = FileDialog(hwnd_, true, L"Export results", L"CSV results (*.csv)\0*.csv\0All files\0*.*\0", suggested, settings_.last_directory);
    if (!chosen)
    {
        return;
    }
    std::wstring base = *chosen;
    if (win32ui::HasFileExtension(base, L".csv"))
    {
        base.resize(base.size() - 4);
    }
    std::wstring existing;
    int existing_count = 0;
    for (const std::wstring& path : {base + L".csv", base + L".json"})
    {
        if (!win32ui::EqualsInsensitive(path, *chosen) && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            existing += (existing.empty() ? L"" : L" and ") + win32ui::FileName(path);
            ++existing_count;
        }
    }
    if (existing_count > 0)
    {
        win32ui::MessageSpec spec;
        spec.title = L"Export results";
        spec.message = existing + (existing_count == 1 ? L" already exists. Replace it?" : L" already exist. Replace them?");
        spec.buttons = {{IDYES, L"&Replace"}, {IDCANCEL, L"Cancel"}};
        spec.default_id = IDCANCEL;
        spec.cancel_id = IDCANCEL;
        if (ui::ShowMessage(hwnd_, std::move(spec)) != IDYES || !hwnd_)
        {
            return;
        }
    }
    engine::ExportInput input;
    input.config = &run_config_;
    input.snapshot = snapshot_.get();
    input.topology = &environment_.topology;
    // export uses environment captured at Start, query again only if missing
    if (!snapshot_ || !snapshot_->environment)
    {
        input.environment = platform::QueryEnvironment();
    }
    input.app_version = THREAD_SIM_VERSION_TEXT;
    const platform::Status status = engine::ExportResults(base, input);
    if (!status)
    {
        ShowError(hwnd_, L"Export results", L"The results could not be exported.", status.error());
        return;
    }
    settings_.last_directory = win32ui::ParentDirectory(base);
    SetStatusMessage(L"Exported " + win32ui::FileName(base) + L".csv and .json");
}

}
