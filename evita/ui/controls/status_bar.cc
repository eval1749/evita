// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/ui/controls/status_bar.h"

#include <commctrl.h>
#include <algorithm>

#include "base/logging.h"
#include "common/win/win32_verify.h"

#pragma comment(lib, "comctl32.lib")

extern HINSTANCE g_hInstance;

namespace ui {

StatusBar::StatusBar() : hwnd_(nullptr), is_simple_mode_(true) {
}

StatusBar::~StatusBar() {
}

void StatusBar::Realize(HWND hwndParent) {
  DCHECK(!hwnd_);
  hwnd_ = ::CreateWindowEx(
      0,
      STATUSCLASSNAMEW,
      nullptr,
      WS_CHILD | WS_VISIBLE,
      0, 0, 0, 0,
      hwndParent,
      reinterpret_cast<HMENU>(this),
      g_hInstance,
      nullptr);
  WIN32_VERIFY(hwnd_);
  RECT rect;
  WIN32_VERIFY(::GetWindowRect(hwnd_, &rect));
  bounds_ = Rect(rect);
  DCHECK(!bounds_.empty());
}

void StatusBar::SetBounds(const common::win::Rect& rect) {
  DCHECK(hwnd_);
  DCHECK(!rect.empty());
  bounds_ = rect;
  ::SetWindowPos(hwnd_, nullptr, rect.left(), rect.top(), rect.width(),
                 rect.height(), SWP_NOZORDER);
}

static int PartWidth(const base::string16& text) {
  auto const kCharWidth = 7;
  auto const kMarginWidth = 6;
  return static_cast<int>(text.length() * kCharWidth + kMarginWidth);
}

void StatusBar::Set(const std::vector<base::string16>& new_texts) {
  DCHECK(hwnd_);
  if (new_texts.size() <= 1) {
    SetSimpleMode(true);
    auto const new_text = new_texts.empty() ? base::string16() : new_texts[0];
    if (simple_text_ != new_text) {
      ::SendMessage(hwnd_, SB_SETTEXT, SB_SIMPLEID | SBT_NOBORDERS,
            reinterpret_cast<LPARAM>(new_text.c_str()));
      simple_text_ = new_text;
    }
    return;
  }

  SetSimpleMode(false);
  if (new_texts == part_texts_)
    return;

  auto const num_columns = new_texts.size();
  if (column_widths_.size() != num_columns) {
    column_widths_.clear();
    column_widths_.resize(num_columns);
  }
  for (auto index = 0u; index < num_columns; ++index) {
    column_widths_[index] = std::max(PartWidth(new_texts[index]),
                                     column_widths_[index]);
  }

  std::vector<int> new_rights(num_columns);
  auto const size_grip_width = ::GetSystemMetrics(SM_CXVSCROLL);
  auto right = bounds_.right() - size_grip_width;
  for (auto index = num_columns;  index > 0; --index) {
    new_rights[index - 1] = right;
    right -= column_widths_[index - 1];
  }

  if (part_rights_ != new_rights) {
    part_rights_.resize(new_rights.size());
    ::SendMessage(hwnd_, SB_SETPARTS, new_rights.size(),
        reinterpret_cast<LPARAM>(new_rights.data()));
  }

  part_texts_.resize(num_columns);
  for (auto index = 0u; index < num_columns;  ++index) {
    if (part_rights_[index] == new_rights[index] &&
        part_texts_[index] == new_texts[index]) {
      continue;
    }
    ::SendMessage(hwnd_, SB_SETTEXT,
                  static_cast<WPARAM>(index | SBT_NOBORDERS),
                  reinterpret_cast<LPARAM>(new_texts[index].c_str()));
    part_texts_[index] = new_texts[index];
    part_rights_[index] = new_rights[index];
  }
}

void StatusBar::Set(const base::string16& text) {
  Set(std::vector<base::string16> {text});
}

void StatusBar::SetPart(size_t part, const base::string16& new_text) {
  DCHECK(hwnd_);

  if (is_simple_mode_) {
    Set(new_text);
    return;
  }

  SetSimpleMode(false);

  if (part_texts_[static_cast<size_t>(part)] == new_text)
    return;

  ::SendMessage(hwnd_, SB_SETTEXT,
                static_cast<WPARAM>(part | SBT_NOBORDERS),
                reinterpret_cast<LPARAM>(new_text.c_str()));
  part_texts_[part] = new_text;
}

void StatusBar::SetSimpleMode(bool new_simple_mode) {
  if (new_simple_mode == is_simple_mode_)
    return;
  if (is_simple_mode_) {
    part_texts_.clear();
    part_rights_.clear();
  } else {
    simple_text_.clear();
  }
  ::SendMessage(hwnd_, SB_SIMPLE, static_cast<WPARAM>(new_simple_mode), 0);
  is_simple_mode_ = new_simple_mode;
}

}  // namespace ui
