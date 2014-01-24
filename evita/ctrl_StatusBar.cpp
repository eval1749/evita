#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Status Bar
// listener/winapp/vi_statusBar.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_StatusBar.cpp#1 $
//
#define DEBUG_STATUSBAR 0
#include "evita/ctrl_StatusBar.h"

#include <algorithm>

#include "base/logging.h"
#include "common/win/win32_verify.h"
#include "evita/vi_defs.h"

#pragma comment(lib, "comctl32.lib")

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
  WIN32_VERIFY(::GetWindowRect(hwnd_, &rect_));
  DCHECK(!rect_.is_empty());
}

void StatusBar::ResizeTo(const common::win::Rect& rect) {
  DCHECK(hwnd_);
  DCHECK(!rect.is_empty());
  rect_ = rect;
  ::SetWindowPos(hwnd_, nullptr, rect.left, rect.top, rect.width(),
                 rect.height(), SWP_NOZORDER);
}

static int PartWidth(const base::string16& text) {
  auto const kCharWidth = 10;
  return static_cast<int>(text.length() * kCharWidth + 10);
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
  std::vector<int> new_rights(new_texts.size());
  auto right = rect_.right;
  for (auto index = new_texts.size();  index > 0; --index) {
    new_rights[index - 1] = right;
    right -= PartWidth(new_texts[index - 1]);
  }

  if (part_rights_ != new_rights) {
    part_rights_.resize(new_rights.size());
    ::SendMessage(hwnd_, SB_SETPARTS, new_rights.size(),
        reinterpret_cast<LPARAM>(new_rights.data()));
  }

  part_texts_.resize(new_texts.size());
  for (auto index = 0u; index < new_texts.size();  ++index) {
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
