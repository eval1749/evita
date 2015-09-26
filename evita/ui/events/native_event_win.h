// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_
#define EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_

#include "base/event_types.h"
#include "evita/gfx/rect.h"

#if !defined(GET_X_LPARAM)
#define GET_X_LPARAM(lParam) \
  (static_cast<int>(static_cast<int16_t>(LOWORD(lParam))))
#define GET_Y_LPARAM(lParam) \
  (static_cast<int>(static_cast<int16_t>(HIWORD(lParam))))
#endif

namespace ui {
gfx::Point GetClientPointFromNativeEvent(const base::NativeEvent& native_event);
gfx::Point GetScreenPointFromNativeEvent(const base::NativeEvent& native_event);
}  // namespace ui

#endif  // EVITA_UI_EVENTS_NATIVE_EVENT_WIN_H_
