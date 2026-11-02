// Copyright (C) 2026 nohuto
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "ui/dialog.h"

#include "engine/config.h"
#include "engine/snapshot.h"

#include <win32ui/list_view/sort.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace thread_sim::frame
{

struct DetailsContent
{
    const engine::ThreadConfig* requested = nullptr;
    const engine::RunSnapshot* run = nullptr;
    const engine::ThreadSnapshot* observed = nullptr;
};

class DetailsWindow final : public win32ui::DialogWindow
{
  public:
    explicit DetailsWindow(uint64_t thread_key)
        : thread_key_(thread_key)
    {
    }
    uint64_t thread_key() const noexcept
    {
        return thread_key_;
    }
    bool Open(HWND owner, const DetailsContent& content);
    void Show(const DetailsContent& content);

  protected:
    void OnCreate() override;
    void OnLayout(int width, int height) override;
    LRESULT OnNotify(NMHDR* header, bool* handled) override;
    HWND initial_focus() const override
    {
        return list_;
    }

  private:
    void SetRows(win32ui::TextRows rows);
    bool MeasureColumns(bool keep);
    void FitColumns();

    uint64_t thread_key_ = 0;
    // during Open only
    const DetailsContent* opening_ = nullptr;
    std::wstring title_;
    HWND list_ = nullptr;
    win32ui::TextRows rows_;
    std::array<int, 4> wanted_ = {};
    // dpi of wanted_, 0 = not measured yet
    UINT wanted_dpi_ = 0;
    int sort_column_ = -1;
    bool sort_ascending_ = true;
    win32ui::SortKeys sort_earlier_;
};

}
