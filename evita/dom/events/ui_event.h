// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_UI_EVENT_H_
#define EVITA_DOM_EVENTS_UI_EVENT_H_

#include "evita/dom/events/event.h"

namespace dom {

class UiEventInit;
class ViewEventTarget;

namespace bindings {
class UiEventClass;
}

class UiEvent : public v8_glue::Scriptable<UiEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(UiEvent)

 public:
  // Exposed for "idle" event.
  UiEvent(const base::string16& type, const UiEventInit& init_dict);
  ~UiEvent() override;

  int detail() const { return detail_; }
  ViewEventTarget* view() const { return view_.get(); }

 private:
  friend class bindings::UiEventClass;

  explicit UiEvent(const base::string16& type);

  int detail_;
  gc::Member<ViewEventTarget> view_;

  DISALLOW_COPY_AND_ASSIGN(UiEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_UI_EVENT_H_
