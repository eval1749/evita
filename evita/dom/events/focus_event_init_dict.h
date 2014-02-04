// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_focus_event_init_dict_h)
#define INCLUDE_evita_dom_events_focus_event_init_dict_h

#include "evita/dom/events/ui_event_init_dict.h"

#include "evita/gc/member.h"

namespace dom {

class EventTarget;

class FocusEventInitDict : public UiEventInitDict {
  private: gc::Member<EventTarget> related_target_;

  public: FocusEventInitDict();
  public: virtual ~FocusEventInitDict();

  public: EventTarget* related_target() const {
    return related_target_.get();
  }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::String> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(FocusEventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_focus_event_init_dict_h)
