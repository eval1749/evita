// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_UI_CONTROLS_STATUS_BAR_H_
#define EVITA_UI_CONTROLS_STATUS_BAR_H_

#include <windows.h>

#include <vector>

#include "base/strings/string16.h"
#include "common/win/rect.h"

namespace ui {

using common::win::Rect;

//////////////////////////////////////////////////////////////////////
//
// StatusBar
//
class StatusBar final {
 public:
  StatusBar();
  ~StatusBar();

  explicit operator bool() const { return hwnd_ != nullptr; }

  const Rect& bounds() const { return bounds_; }
  // TODO(eval1749) We should get rid of |StatusBar::height()|.
  int height() const { return bounds_.height(); }
  HWND hwnd() const { return hwnd_; }

  void Realize(HWND hwndParent);
  void SetBounds(const common::win::Rect& rect);
  void Set(const std::vector<base::string16>& texts);
  void Set(const base::string16& text);
  void SetPart(size_t part, const base::string16& text);
  void SetSimpleMode(bool simple_mode);

 private:
  HWND hwnd_;
  bool is_simple_mode_;
  std::vector<int> part_rights_;
  std::vector<int> column_widths_;
  std::vector<base::string16> part_texts_;
  common::win::Rect bounds_;
  base::string16 simple_text_;

  DISALLOW_COPY_AND_ASSIGN(StatusBar);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_STATUS_BAR_H_
