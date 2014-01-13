// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_window_event_h)
#define INCLUDE_evita_dom_events_window_event_h

#include "evita/dom/events/event.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class Window;

class WindowEvent : public v8_glue::Scriptable<WindowEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(WindowEvent)

  private: gc::Member<Window> source_window_;

  public: WindowEvent(const base::string16& type, BubblingType bubbles,
                      CancelableType cancelable, Window* source_window);
  public: WindowEvent();
  public: virtual ~WindowEvent();

  public: Nullable<Window> source_window() const {
    return source_window_.get();
  }

  public: void InitWindowEvent(const base::string16& type,
                               BubblingType bubbles, CancelableType cancelable,
                               Window* source_window);

  DISALLOW_COPY_AND_ASSIGN(WindowEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_window_event_h)
