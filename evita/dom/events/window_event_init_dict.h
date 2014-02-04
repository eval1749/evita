// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_window_event_init_dict_h)
#define INCLUDE_evita_dom_events_window_event_init_dict_h

#include "evita/dom/events/event_init_dict.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class WindowEventInitDict : public EventInitDict {
  private: gc::Member<Window> source_window_;

  public: WindowEventInitDict();
  public: virtual ~WindowEventInitDict();

  public: Window* source_window() const {
    return source_window_.get();
  }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::String> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(WindowEventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_window_event_init_dict_h)
