// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/tooltip.h"

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "base/logging.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Tooltip::ToolDelegate
//
Tooltip::ToolDelegate::ToolDelegate() {
}

Tooltip::ToolDelegate::~ToolDelegate() {
}

//////////////////////////////////////////////////////////////////////
//
// Tooltip
//
Tooltip::Tooltip()
    : tool_hwnd_(nullptr),
      tooltip_hwnd_(nullptr) {
}

Tooltip::~Tooltip() {
  if (!tooltip_hwnd_)
    ::DestroyWindow(tooltip_hwnd_);
}

void Tooltip::AddTool(ToolDelegate* tool_delegate) {
  TOOLINFO info = {0};
  info.lpszText = LPSTR_TEXTCALLBACK;
  info.uId = reinterpret_cast<UINT_PTR>(tool_delegate);
  SendMessage(TTM_ADDTOOL, &info);
}

void Tooltip::DeleteTool(ToolDelegate* tool_delegate) {
  if (!tooltip_hwnd_)
    return;
  TOOLINFO info = {0};
  info.uId = reinterpret_cast<UINT_PTR>(tool_delegate);
  SendMessage(TTM_DELTOOL, &info);
}

void Tooltip::OnNotify(NMHDR* nmhdr) {
  if (nmhdr->hwndFrom != tooltip_hwnd_ || nmhdr->code != TTN_NEEDTEXT)
    return;
  // Set width of tool tip
  ::SendMessage(tooltip_hwnd_, TTM_SETMAXTIPWIDTH, 0, 300);
  auto const disp_info = reinterpret_cast<NMTTDISPINFO*>(nmhdr);
  tool_text_ = reinterpret_cast<ToolDelegate*>(nmhdr->idFrom)->
      GetTooltipText();
  disp_info->lpszText = const_cast<LPWSTR>(tool_text_.c_str());
}

void Tooltip::Realize(HWND tool_hwnd) {
  DCHECK(tool_hwnd);
  DCHECK(!tool_hwnd_);
  DCHECK(!tooltip_hwnd_);
  tool_hwnd_ = tool_hwnd;
  tooltip_hwnd_ = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
      WS_POPUP | TTS_NOPREFIX, 0, 0, 0, 0, tool_hwnd, nullptr, nullptr,
      nullptr);
  if (!tooltip_hwnd_) {
    PLOG(WARNING) << "tooltipe creation failed, disabling tooltips";
    return;
  }

  TOOLINFO info;
  info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  info.uId = reinterpret_cast<UINT_PTR>(tool_hwnd_);
  info.lpszText = nullptr;
  SendMessage(TTM_ADDTOOL, &info);
}

void Tooltip::SendMessage(UINT message, TOOLINFO* info) {
  info->cbSize = sizeof(*info);
  info->hwnd = tool_hwnd_;
  ::SendMessage(tooltip_hwnd_, message, 0, reinterpret_cast<LPARAM>(info));
}

void Tooltip::SetToolBounds(ToolDelegate* tool, const gfx::Rect& bounds) {
  TOOLINFO info = {0};
  info.rect = static_cast<RECT>(bounds);
  info.uId = reinterpret_cast<UINT_PTR>(tool);
  SendMessage(TTM_NEWTOOLRECT, &info);
}

}  // namespace ui
