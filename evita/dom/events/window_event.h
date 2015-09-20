// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_WINDOW_EVENT_H_
#define EVITA_DOM_EVENTS_WINDOW_EVENT_H_

#include "evita/dom/events/event.h"

namespace dom {

class Window;
class WindowEventInit;

namespace bindings {
class WindowEventClass;
}

class WindowEvent final : public v8_glue::Scriptable<WindowEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(WindowEvent)

 public:
  WindowEvent(const base::string16& type, const WindowEventInit& init_dict);
  ~WindowEvent() final;

 private:
  friend class bindings::WindowEventClass;

  explicit WindowEvent(const base::string16& type);

  Window* source_window() const { return source_window_.get(); }

  gc::Member<Window> source_window_;

  DISALLOW_COPY_AND_ASSIGN(WindowEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_WINDOW_EVENT_H_
