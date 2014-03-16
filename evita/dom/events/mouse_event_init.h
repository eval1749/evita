// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_mouse_event_init_h)
#define INCLUDE_evita_dom_events_mouse_event_init_h

#include "evita/dom/events/ui_event_init.h"

#include "evita/dom/public/view_event.h"
#include "evita/gc/member.h"

namespace domapi {
struct MouseEvent;
}

namespace dom {

class EventTarget;

class MouseEventInit : public UiEventInit {
  private: bool alt_key_;
  private: int button_;
  private: int buttons_;
  private: int client_x_;
  private: int client_y_;
  private: bool ctrl_key_;
  private: bool meta_key_;
  private: bool shift_key_;

  public: explicit MouseEventInit(const domapi::MouseEvent& api_event);
  public: MouseEventInit();
  public: virtual ~MouseEventInit();

  public: bool alt_key() const { return alt_key_; }
  public: int button() const { return button_; }
  public: int buttons() const { return buttons_; }
  public: int client_x() const { return client_x_; }
  public: int client_y() const { return client_y_; }
  public: bool ctrl_key() const { return ctrl_key_; }
  public: bool meta_key() const { return meta_key_; }
  public: bool shift_key() const { return shift_key_; }

  // dom::Dictionary
  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_mouse_event_init_h)
