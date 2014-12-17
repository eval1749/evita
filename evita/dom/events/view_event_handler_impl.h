// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_view_event_handler_impl_h)
#define INCLUDE_evita_dom_events_view_event_handler_impl_h

#include "evita/dom/public/view_event_handler.h"

namespace dom {

class Event;
class EventTarget;
class ScriptHost;

class ViewEventHandlerImpl final : public domapi::ViewEventHandler {
  private: ScriptHost* host_;

  public: ViewEventHandlerImpl(ScriptHost* host);
  public: ~ViewEventHandlerImpl() final;

  public: void CallClassEventHandler(EventTarget* event_target,
                                     Event* event);
  private: void DispatchEventWithInLock(EventTarget* event_target,
                                        Event* event);

  // domapi::ViewEventHandler
  private: void DidChangeWindowBounds(
      WindowId window_id, int left, int top, int right, int bottom) final;
  private: void DidChangeWindowVisibility(
      WindowId window_id, domapi::Visibility visibility) final;
  private: void DidDestroyWidget(WindowId window_id) final;
  private: void DidDropWidget(WindowId source_id, WindowId target_id) final;
  private: void DidRealizeWidget(WindowId window_id) final;
  private: void DidStartViewHost() final;
  private: void DispatchFocusEvent(const domapi::FocusEvent& event) final;
  private: void DispatchKeyboardEvent(const domapi::KeyboardEvent& event) final;
  private: void DispatchMouseEvent(const domapi::MouseEvent& event) final;
  private: void DispatchTextCompositionEvent(
      const domapi::TextCompositionEvent& event) final;
  private: void DispatchWheelEvent(const domapi::WheelEvent& event) final;
  private: void OpenFile(WindowId window_id,
                         const base::string16& file_name) final;
  private: void QueryClose(WindowId window_id) final;
  private: void RunCallback(base::Closure callback) final;
  private: void WillDestroyHost() final;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandlerImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_view_event_handler_impl_h)
