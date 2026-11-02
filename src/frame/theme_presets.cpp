// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "frame/main_window.h"

#include "frame/window_detail.h"

#include "ui/dialog.h"
#include "ui/theme.h"

#include <win32ui/dialog/text_prompt.h>
#include <win32ui/theme/preset_editor.h>

namespace thread_sim::frame
{

using namespace window_detail;

namespace
{

constexpr wchar_t kPresetFile[] = L"theme_presets.rktheme";

void ShowPresetProblem(HWND owner, std::wstring_view message, const std::wstring& path, const win32ui::ThemePresetResult& result)
{
    const std::wstring problem = win32ui::ThemePresetErrorText(result);
    win32ui::MessageSpec spec;
    spec.title = L"ThreadSim";
    spec.message = message;
    spec.details = (problem.empty() ? std::wstring() : problem + L"\n\n") + L"File: " + path;
    spec.buttons = {{IDOK, L"OK"}};
    spec.cancel_id = IDOK;
    spec.icon = IDI_WARNING;
    ui::ShowMessage(owner, std::move(spec));
}

}

std::wstring MainWindow::ThemePresetsPath() const
{
    return environment_.data_dir.empty() ? std::wstring() : environment_.data_dir + L"\\" + kPresetFile;
}

void MainWindow::LoadThemePresets()
{
    if (presets_loaded_)
    {
        return;
    }
    presets_loaded_ = true;
    const std::wstring path = ThemePresetsPath();
    if (path.empty())
    {
        theme_presets_ = win32ui::BuiltInThemePresets();
        return;
    }
    const win32ui::ThemePresetLoad load = win32ui::LoadThemePresetFile(path, &theme_presets_, &settings_.theme_preset);
    if (load.result.error != win32ui::ThemePresetError::none)
    {
        presets_problem_ = load.result;
    }
    if (load.save_needed)
    {
        SaveThemePresets();
    }
}

void MainWindow::ShowThemePresetsProblem()
{
    if (hwnd_ && presets_problem_)
    {
        const win32ui::ThemePresetResult problem = *presets_problem_;
        presets_problem_.reset();
        ShowPresetProblem(hwnd_, L"The theme presets file could not be read, the built-in presets are used until the presets are saved.", ThemePresetsPath(), problem);
    }
    if (hwnd_ && presets_save_problem_)
    {
        const win32ui::ThemePresetResult problem = *presets_save_problem_;
        presets_save_problem_.reset();
        ShowPresetProblem(hwnd_, L"The theme presets could not be saved.", ThemePresetsPath(), problem);
    }
}

void MainWindow::SaveThemePresets()
{
    const std::wstring path = ThemePresetsPath();
    if (path.empty())
    {
        return;
    }
    const win32ui::ThemePresetResult result = win32ui::WriteThemePresets(path, theme_presets_);
    if (result.error == win32ui::ThemePresetError::none)
    {
        return;
    }
    if (hwnd_)
    {
        ShowPresetProblem(hwnd_, L"The theme presets could not be saved.", path, result);
    }
    else
    {
        presets_save_problem_ = result;
    }
}

void MainWindow::ApplyThemePreset(const std::wstring& name)
{
    LoadThemePresets();
    const win32ui::ThemePreset* preset = win32ui::FindThemePreset(theme_presets_, name);
    settings_.theme = win32ui::ThemeMode::custom;
    settings_.theme_preset = preset ? preset->name : name;
    win32ui::SetThemeMode(win32ui::ThemeMode::custom, preset ? &preset->colors : nullptr);
    if (hwnd_)
    {
        ApplyTheme();
        ShowThemePresetsProblem();
    }
}

void MainWindow::ShowThemePresets()
{
    LoadThemePresets();
    ShowThemePresetsProblem();
    win32ui::PresetEditorHooks hooks;
    hooks.prompt_name = [](HWND owner, const wchar_t* title, const std::wstring& initial, std::wstring* name) {
        *name = initial;
        return win32ui::PromptText(owner, {.title = title, .label = L"Name"}, name) == 0;
    };
    hooks.confirm_delete = [](HWND owner, const std::wstring& name) {
        win32ui::MessageSpec spec;
        spec.title = L"ThreadSim";
        spec.message = L"Delete the theme preset " + name + L"?";
        spec.buttons = {{IDYES, L"&Delete"}, {IDCANCEL, L"Cancel"}};
        spec.default_id = IDCANCEL;
        spec.cancel_id = IDCANCEL;
        return ui::ShowMessage(owner, std::move(spec)) == IDYES;
    };
    hooks.import_presets = [](HWND owner, std::vector<win32ui::ThemePreset>* presets) {
        const std::optional<std::wstring> path = FileDialog(owner, false, L"Import theme presets", L"Theme presets\0*.rktheme\0All files\0*.*\0", L"", L"");
        if (!path)
        {
            return false;
        }
        const win32ui::ThemePresetResult result = win32ui::ReadThemePresets(*path, presets);
        if (result.error == win32ui::ThemePresetError::none && !presets->empty())
        {
            return true;
        }
        ShowPresetProblem(owner, L"The file has no theme presets ThreadSim can read.", *path, result);
        return false;
    };
    hooks.export_presets = [](HWND owner, const std::vector<win32ui::ThemePreset>& presets) {
        const std::optional<std::wstring> path = FileDialog(owner, true, L"Export theme presets", L"Theme presets\0*.rktheme\0All files\0*.*\0", kPresetFile, L"");
        if (!path)
        {
            return;
        }
        if (const win32ui::ThemePresetResult result = win32ui::WriteThemePresets(*path, presets); result.error != win32ui::ThemePresetError::none)
        {
            ShowPresetProblem(owner, L"The theme presets could not be exported.", *path, result);
        }
    };
    hooks.apply = [](void* context, const std::vector<win32ui::ThemePreset>& presets, const std::wstring& active_name) {
        auto* window = static_cast<MainWindow*>(context);
        window->theme_presets_ = presets;
        window->SaveThemePresets();
        window->ApplyThemePreset(active_name);
        window->SaveSettings();
        window->UpdateCommandStates();
    };
    hooks.context = this;
    win32ui::ShowThemePresetEditor(hwnd_, theme_presets_, settings_.theme_preset, &hooks);
}

}
