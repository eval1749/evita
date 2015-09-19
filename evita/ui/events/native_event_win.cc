// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "common/win/win32_verify.h"
#include "evita/ui/events/native_event_win.h"

namespace ui {

gfx::Point GetClientPointFromNativeEvent(
    const base::NativeEvent& native_event) {
  auto const message = native_event.message;
  if (message == WM_MOUSEWHEEL ||
      (message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK)) {
    auto client_point(native_event.pt);
    WIN32_VERIFY(
        ::MapWindowPoints(HWND_DESKTOP, native_event.hwnd, &client_point, 1));
    return gfx::Point(client_point);
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
    return gfx::Point(native_event.pt);

  NOTREACHED();
  return gfx::Point();
}

gfx::Point GetScreenPointFromNativeEvent(
    const base::NativeEvent& native_event) {
  auto const message = native_event.message;
  if (message == WM_MOUSEWHEEL ||
      (message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK)) {
    return gfx::Point(native_event.pt);
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    auto screen_point(native_event.pt);
    WIN32_VERIFY(
        ::MapWindowPoints(native_event.hwnd, HWND_DESKTOP, &screen_point, 1));
    return gfx::Point(screen_point);
  }
  NOTREACHED();
  return gfx::Point();
}

}  // namespace ui
