// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_mouse_event_h)
#define INCLUDE_evita_dom_events_mouse_event_h

#include "base/strings/string16.h"
#include "evita/dom/events/ui_event.h"

namespace domapi {
struct MouseEvent;
}

namespace dom {

class MouseEventInit;

namespace bindings {
class MouseEventClass;
}

class MouseEvent : public v8_glue::Scriptable<MouseEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(MouseEvent);
  friend class bindings::MouseEventClass;

  private: bool alt_key_;
  private: int button_;
  private: int buttons_;
  private: int client_x_;
  private: int client_y_;
  private: bool ctrl_key_;
  private: bool meta_key_;
  private: bool shift_key_;

  protected: MouseEvent(const base::string16& type,
                        const MouseEventInit& init_dict);
  private: MouseEvent(const base::string16& type);
  public: explicit MouseEvent(const domapi::MouseEvent& api_event);
  public: virtual ~MouseEvent();

  private: bool alt_key() const { return alt_key_; }
  private: int button() const { return button_; }
  private: int buttons() const { return buttons_; }
  private: int client_x() const { return client_x_; }
  private: int client_y() const { return client_y_; }
  private: bool ctrl_key() const { return ctrl_key_; }
  private: bool meta_key() const { return meta_key_; }
  private: bool shift_key() const { return shift_key_; }

  DISALLOW_COPY_AND_ASSIGN(MouseEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_mouse_event_h)
