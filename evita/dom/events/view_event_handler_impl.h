// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_view_event_handler_impl_h)
#define INCLUDE_evita_dom_events_view_event_handler_impl_h

#include "evita/dom/public/view_event_handler.h"

namespace dom {

class Event;
class EventTarget;
class ScriptController;

class ViewEventHandlerImpl : public domapi::ViewEventHandler {
  private: ScriptController* controller_;

  public: ViewEventHandlerImpl(ScriptController* controller);
  public: ~ViewEventHandlerImpl();

  private: void DispatchEvent(EventTarget* event_target, Event* event);

  // domapi::ViewEventHandler
  private: virtual void AppendTextToBuffer(text::Buffer* buffer,
                                           const base::string16& text) override;
  private: virtual void DidDestroyWidget(WindowId window_id) override;
  private: virtual void DidDropWidget(WindowId source_id,
                                      WindowId target_id) override;
  private: virtual void DidKillFocus(WindowId window_id) override;
  private: virtual void DidRealizeWidget(WindowId window_id) override;
  private: virtual void DidResizeWidget(WindowId window_id, int left, int top,
                                       int right, int bottom) override;
  private: virtual void DidRequestFocus(WindowId window_id) override;
  private: virtual void DidStartHost() override;
  private: virtual void DispatchFormEvent(
      const domapi::FormEvent& event) override;
  private: virtual void DispatchKeyboardEvent(
      const domapi::KeyboardEvent& event) override;
  private: virtual void DispatchMouseEvent(
      const domapi::MouseEvent& event) override;
  private: virtual void DispatchWheelEvent(
      const domapi::WheelEvent& event) override;
  private: virtual void OpenFile(WindowId window_id,
                                 const base::string16& filename) override;
  private: virtual void QueryClose(WindowId window_id) override;
  private: virtual void RunCallback(base::Closure callback) override;
  private: virtual void WillDestroyHost() override;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandlerImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_view_event_handler_impl_h)
