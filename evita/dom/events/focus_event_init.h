// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_focus_event_init_h)
#define INCLUDE_evita_dom_events_focus_event_init_h

#include "evita/dom/events/ui_event_init.h"

#include "evita/gc/member.h"

namespace dom {

class EventTarget;

class FocusEventInit : public UiEventInit {
  private: gc::Member<EventTarget> related_target_;

  public: FocusEventInit();
  public: virtual ~FocusEventInit();

  public: EventTarget* related_target() const {
    return related_target_.get();
  }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(FocusEventInit);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_focus_event_init_h)
