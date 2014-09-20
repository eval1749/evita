// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_events_native_event_win_h)
#define INCLUDE_evita_ui_events_native_event_win_h

#include "base/event_types.h"
#include "evita/gfx/rect.h"

#if !defined(GET_X_LPARAM)
#define GET_X_LPARAM(lParam) ((int)(short)LOWORD(lParam))
#define GET_Y_LPARAM(lParam) ((int)(short)HIWORD(lParam))
#endif

namespace ui {
gfx::Point GetClientPointFromNativeEvent(const base::NativeEvent& native_event);
gfx::Point GetScreenPointFromNativeEvent(const base::NativeEvent& native_event);
}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_events_native_event_win_h)
