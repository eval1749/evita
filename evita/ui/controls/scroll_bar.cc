// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/scroll_bar.h"

#include "evita/ui/controls/scroll_bar_observer.h"

extern HINSTANCE g_hInstance;

namespace ui {

ScrollBar::ScrollBar(Type type, ScrollBarObserver* observer)
    : hwnd_(nullptr), observer_(observer), type_(type) {
}

ScrollBar::~ScrollBar() {
  if (hwnd_)
    ::DestroyWindow(hwnd_);
}

void ScrollBar::SetData(const Data& data) {
  if (!hwnd_)
    return;
  SCROLLINFO info;
  info.cbSize = sizeof(info);
  info.fMask = SIF_POS | SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
  info.nPage = static_cast<UINT>(data.thumb_size);
  info.nMin = data.minimum;
  info.nMax = data.maximum;
  info.nPos = data.thumb_value;
  ::SetScrollInfo(hwnd_, SB_CTL, &info, true);
}

// ui::Widget
void ScrollBar::DidChangeHierarchy() {
  if (!hwnd_)
    return;
  ::SetParent(hwnd_, AssociatedHwnd());
}

void ScrollBar::DidHide() {
  if (!hwnd_)
    return;
  ::ShowWindow(hwnd_, SW_HIDE);
}

void ScrollBar::DidRealize() {
  DCHECK(!hwnd_);

  auto const style = static_cast<DWORD>(WS_CHILD | WS_VISIBLE |
      (type_ == Type::Vertical ? SBS_VERT : SBS_HORZ));

  hwnd_ = CreateWindowEx(0, L"SCROLLBAR", L"", style, 0, 0, 0, 0,
                         AssociatedHwnd(), reinterpret_cast<HMENU>(this),
                         g_hInstance, nullptr);
}


void ScrollBar::DidResize() {
  if (!hwnd_)
    return;
  DCHECK(!bounds().empty());
  ::SetWindowPos(hwnd_, nullptr, bounds().left, bounds().top, bounds().width(),
                 bounds().height(), SWP_NOZORDER);
}

void ScrollBar::DidShow() {
  if (!hwnd_)
    return;
  ::ShowWindow(hwnd_, SW_SHOW);
}

void ScrollBar::OnScroll(int request) {
  switch (request) {
    case SB_LINEDOWN: // 1
      observer_->DidClickLineDown();
      break;

    case SB_LINEUP: // 0
      observer_->DidClickLineUp();
      break;

    case SB_PAGEDOWN: // 3
      observer_->DidClickPageDown();
      break;

    case SB_PAGEUP: // 2
      observer_->DidClickPageUp();
      break;

    case SB_THUMBTRACK: { // 5
      SCROLLINFO info;
      info.cbSize = sizeof(info);
      info.fMask = SIF_ALL;
      if (::GetScrollInfo(hwnd_, SB_CTL, &info))
        observer_->DidMoveThumb(info.nTrackPos);
      break;
    }
  }
}

}  // namespace ui
