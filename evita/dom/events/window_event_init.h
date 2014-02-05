// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_window_event_init_h)
#define INCLUDE_evita_dom_events_window_event_init_h

#include "evita/dom/events/event_init.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class WindowEventInit : public EventInit {
  private: gc::Member<Window> source_window_;

  public: WindowEventInit();
  public: virtual ~WindowEventInit();

  public: Window* source_window() const {
    return source_window_.get();
  }

  public: void set_source_window(Window* source_window) {
    source_window_ = source_window;
  }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(WindowEventInit);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_window_event_init_h)
