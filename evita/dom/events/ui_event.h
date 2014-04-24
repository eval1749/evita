// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_ui_event_h)
#define INCLUDE_evita_dom_events_ui_event_h

#include "evita/dom/events/event.h"

namespace dom {

class UiEventInit;
class ViewEventTarget;

namespace bindings {
class UiEventClass;
}

class UiEvent : public v8_glue::Scriptable<UiEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(UiEvent)
  friend class bindings::UiEventClass;

  private: int detail_;
  private: gc::Member<ViewEventTarget> view_;

  // Exposed for "idle" event.
  public: UiEvent(const base::string16& type, const UiEventInit& init_dict);
  private: UiEvent(const base::string16& type);
  public: virtual ~UiEvent();

  public: int detail() const { return detail_; }
  public: ViewEventTarget* view() const { return view_.get(); }

  DISALLOW_COPY_AND_ASSIGN(UiEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_ui_event_h)
