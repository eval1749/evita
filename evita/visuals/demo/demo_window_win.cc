// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "evita/visuals/demo/demo_application.h"
#include "evita/visuals/demo/demo_window.h"

namespace visuals {

// ui::Widget
void DemoWindow::CreateNativeWindow() const {
  const DWORD dwExStyle =
      WS_EX_APPWINDOW | WS_EX_NOPARENTNOTIFY | WS_EX_WINDOWEDGE;
  const DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;

  RECT window_bounds{0, 0, 800, 600};
  ::AdjustWindowRectEx(&window_bounds, dwStyle, TRUE, dwExStyle);

  native_window()->CreateWindowEx(
      WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE, L"Visuals Demo",
      nullptr, gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      gfx::Size(window_bounds.right - window_bounds.left,
                window_bounds.bottom - window_bounds.top));
}

}  // namespace visuals

//////////////////////////////////////////////////////////////////////
//
// WinMain
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  common::win::NativeWindow::Init(hInstance);
  visuals::DemoApplication().Run();
  return 0;
}
