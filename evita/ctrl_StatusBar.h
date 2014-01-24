//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_StatusBar.h#1 $
//
#if !defined(INCLUDE_visual_statusBar_h)
#define INCLUDE_visual_statusBar_h

#include <vector>

#include "base/strings/string16.h"
#include "common/win/rect.h"

namespace ui {

using common::win::Rect;

//////////////////////////////////////////////////////////////////////
//
// StatusBar
//
class StatusBar {
  private: HWND hwnd_;
  private: bool is_simple_mode_;
  private: std::vector<int> part_rights_;
  private: std::vector<base::string16> part_texts_;
  private: common::win::Rect rect_;
  private: base::string16 simple_text_;

  public: StatusBar();
  public: ~StatusBar();

  public: operator bool() const { return hwnd_; }
  public: int height() const { return rect_.height(); }

  public: void Realize(HWND hwndParent);
  public: void ResizeTo(const common::win::Rect& rect);
  public: void Set(const std::vector<base::string16>& texts);
  public: void Set(const base::string16& text);
  public: void SetPart(size_t part, const base::string16& text);
  public: void SetSimpleMode(bool simple_mode);
};

}  // namespace ui

#endif //!defined(INCLUDE_visual_statusBar_h)
