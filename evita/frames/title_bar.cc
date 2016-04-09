// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/frames/title_bar.h"

#include <algorithm>

#include "base/logging.h"

namespace views {

TitleBar::TitleBar() : hwnd_(nullptr) {}

TitleBar::~TitleBar() {}

void TitleBar::Realize(HWND hwnd) {
  DCHECK(hwnd);
  DCHECK(!hwnd_);
  hwnd_ = hwnd;
}

void TitleBar::SetText(const base::string16& new_title) {
  if (title_ == new_title)
    return;

  if (!::SetWindowTextW(hwnd_, new_title.c_str())) {
    PLOG(ERROR) << "SetWindowText failed.";
    return;
  }
  title_ = new_title;
}

}  // namespace views
