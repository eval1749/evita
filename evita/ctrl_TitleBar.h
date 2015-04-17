// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_visual_control_TitleBar_h)
#define INCLUDE_visual_control_TitleBar_h

#include <windows.h>

#include "base/strings/string16.h"

namespace views {

class TitleBar {
  private: HWND  hwnd_;
  private: base::string16 title_;

  public: TitleBar();
  public: ~TitleBar();

  public: void Realize(HWND hwnd);
  public: void SetText(const base::string16& new_title);
};

}  // namespace views

#endif //!defined(INCLUDE_visual_control_TitleBar_h)
