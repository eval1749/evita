// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_view_event_handler_h)
#define INCLUDE_evita_dom_public_view_event_handler_h

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/public/event_target_id.h"
#include "evita/dom/windows/window_id.h"

namespace text {
class Buffer;
}

namespace domapi {

struct FocusEvent;
struct FormEvent;
struct KeyboardEvent;
struct MouseEvent;
struct TextCompositionEvent;
struct WheelEvent;
using dom::WindowId;

class ViewEventHandler {
  public: ViewEventHandler();
  public: virtual ~ViewEventHandler();

  public: virtual void DidChangeWindowBounds(
      WindowId window_id, int left, int top, int right, int bottom) = 0;
  public: virtual void DidDestroyWidget(WindowId window_id) = 0;
  public: virtual void DidDropWidget(WindowId source_id,
                                     WindowId target_id)  = 0;
  public: virtual void DidRealizeWidget(WindowId window_id) = 0;
  public: virtual void DidStartViewHost() = 0;
  public: virtual void DispatchFocusEvent(const FocusEvent& event) = 0;
  public: virtual void DispatchKeyboardEvent(const KeyboardEvent& event) = 0;
  public: virtual void DispatchMouseEvent(const MouseEvent& event) = 0;
  public: virtual void DispatchTextCompositionEvent(
      const TextCompositionEvent& event) = 0;
  public: virtual void DispatchViewIdleEvent(int hint) = 0;
  public: virtual void DispatchWheelEvent(const WheelEvent& event) = 0;
  public: virtual void OpenFile(WindowId window_id,
                                const base::string16& file_name) = 0;
  public: virtual void QueryClose(WindowId window_id) = 0;
  public: virtual void RunCallback(base::Closure) = 0;
  public: virtual void WillDestroyHost() = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandler);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_view_event_handler_h)
