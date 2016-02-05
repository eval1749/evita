// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_FOCUS_EVENT_H_
#define EVITA_DOM_EVENTS_FOCUS_EVENT_H_

#include "evita/dom/events/ui_event.h"
#include "evita/ginx/nullable.h"

namespace dom {

class FocusEventInit;
class Window;

namespace bindings {
class FocusEventClass;
}

class FocusEvent final : public ginx::Scriptable<FocusEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(FocusEvent)

 public:
  FocusEvent(const base::string16& type, const FocusEventInit& init_dict);
  ~FocusEvent() final;

 private:
  friend class bindings::FocusEventClass;

  explicit FocusEvent(const base::string16& type);

  EventTarget* related_target() const { return related_target_.get(); }

  gc::Member<EventTarget> related_target_;

  DISALLOW_COPY_AND_ASSIGN(FocusEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_FOCUS_EVENT_H_
