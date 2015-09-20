// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_MOUSE_EVENT_H_
#define EVITA_DOM_EVENTS_MOUSE_EVENT_H_

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

 public:
  explicit MouseEvent(const domapi::MouseEvent& api_event);
  ~MouseEvent() override;

 protected:
  MouseEvent(const base::string16& type, const MouseEventInit& init_dict);

 private:
  friend class bindings::MouseEventClass;

  explicit MouseEvent(const base::string16& type);

  bool alt_key() const { return alt_key_; }
  int button() const { return button_; }
  int buttons() const { return buttons_; }
  int client_x() const { return client_x_; }
  int client_y() const { return client_y_; }
  bool ctrl_key() const { return ctrl_key_; }
  bool meta_key() const { return meta_key_; }
  bool shift_key() const { return shift_key_; }

  bool alt_key_;
  int button_;
  int buttons_;
  int client_x_;
  int client_y_;
  bool ctrl_key_;
  bool meta_key_;
  bool shift_key_;

  DISALLOW_COPY_AND_ASSIGN(MouseEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_MOUSE_EVENT_H_
