// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_window.h"

#include "base/logging.h"
#include "evita/gfx_base.h"

#define WIN32_VERIFY(exp) { \
  if (!(exp)) { \
    auto const last_error = ::GetLastError(); \
    DVLOG(0) << "Win32 API failure: " #exp << " error=" << last_error; \
  } \
}

namespace views {

FormWindow::FormWindow(WindowId window_id, const dom::Form* form)
    : views::Window(ui::NativeWindow::Create(*this), window_id),
      form_(form),
      gfx_(new gfx::Graphics()) {
}

FormWindow::~FormWindow() {
}

// ui::Widget
void FormWindow::CreateNativeWindow() const {
  auto const dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
  auto const dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU |WS_VISIBLE;
  // TODO(yosi) We should retrieve size of |FormWindow| from |form|.
  Rect contentRect(0, 0, 400, 240);
  Rect windowRect(contentRect);
  bool has_menu = false;
  WIN32_VERIFY(::AdjustWindowRectEx(&windowRect, dwStyle, has_menu, dwExStyle));

  native_window()->CreateWindowEx(
      // TODO(yosi) We should specify window title of|FormWindow|.
      dwExStyle, dwStyle, L"FormWindow", nullptr,
      gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      windowRect.size());
}

void FormWindow::DidCreateNativeWindow() {
 // TODO(yosi) We should get default value of form window transparency from
 //CSS.
  ::SetLayeredWindowAttributes(*native_window(), RGB(0, 0, 0), 80 * 255 / 100,
                               LWA_ALPHA);
}

}  // namespace views
