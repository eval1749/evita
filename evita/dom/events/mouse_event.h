// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_mouse_event_h)
#define INCLUDE_evita_dom_events_mouse_event_h

#include "base/strings/string16.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/public/api_event.h"

namespace domapi {
struct MouseEvent;
}

namespace dom {

class EventHandler;
class MouseEventInitDict;
class Window;

class MouseEvent : public v8_glue::Scriptable<MouseEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(MouseEvent);

  private: domapi::MouseEvent event_;

  public: MouseEvent(const base::string16& type,
                     const MouseEventInitDict& init_dict);
  public: MouseEvent(const domapi::MouseEvent& api_event);
  public: virtual ~MouseEvent();

  public: bool alt_key() const;
  public: int button() const;
  public: int buttons() const;
  public: int client_x() const;
  public: int client_y() const;
  public: bool ctrl_key() const;
  public: bool shift_key() const;

  DISALLOW_COPY_AND_ASSIGN(MouseEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_mouse_event_h)
