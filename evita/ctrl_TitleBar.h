// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CTRL_TITLEBAR_H_
#define EVITA_CTRL_TITLEBAR_H_

#include <windows.h>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace views {

class TitleBar final {
 public:
  TitleBar();
  ~TitleBar();

  void Realize(HWND hwnd);
  void SetText(const base::string16& new_title);

 private:
  HWND hwnd_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(TitleBar);
};

}  // namespace views

#endif  // EVITA_CTRL_TITLEBAR_H_
