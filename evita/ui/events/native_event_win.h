// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_
#define EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_

#include "evita/gfx/rect.h"
#include "evita/ui/events/platform_event.h"

#if !defined(GET_X_LPARAM)
#define GET_X_LPARAM(lParam) \
  (static_cast<int>(static_cast<int16_t>(LOWORD(lParam))))
#define GET_Y_LPARAM(lParam) \
  (static_cast<int>(static_cast<int16_t>(HIWORD(lParam))))
#endif

namespace ui {
gfx::Point GetClientPointFromNativeEvent(const PlatformEvent& native_event);
gfx::Point GetScreenPointFromNativeEvent(const PlatformEvent& native_event);
}  // namespace ui

#endif  // EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_
