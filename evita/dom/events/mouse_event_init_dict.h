// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_mouse_event_init_dict_h)
#define INCLUDE_evita_dom_events_mouse_event_init_dict_h

#include "evita/dom/events/ui_event_init_dict.h"

#include "evita/gc/member.h"

namespace dom {

class EventTarget;

class MouseEventInitDict : public UiEventInitDict {
  public: MouseEventInitDict();
  public: virtual ~MouseEventInitDict();

  DISALLOW_COPY_AND_ASSIGN(MouseEventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_mouse_event_init_dict_h)
