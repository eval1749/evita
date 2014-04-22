// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_ui_event_h)
#define INCLUDE_evita_dom_events_ui_event_h

#include "evita/dom/events/event.h"
#include "evita/dom/dictionary.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class UiEventInit;
class Window;

class UiEvent : public v8_glue::Scriptable<UiEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(UiEvent)

  private: int detail_;
  private: gc::Member<Window> view_;

  public: UiEvent(const base::string16& type,
                  const UiEventInit& init_dict);
  public: virtual ~UiEvent();

  public: int detail() const { return detail_; }
  public: v8_glue::Nullable<Window> view() const { return view_.get(); }

  DISALLOW_COPY_AND_ASSIGN(UiEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_ui_event_h)
