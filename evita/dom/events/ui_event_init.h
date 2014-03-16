// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_ui_event_init_h)
#define INCLUDE_evita_dom_events_ui_event_init_h

#include "evita/dom/events/event_init.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class UiEventInit : public EventInit {
  private: int detail_;
  private: gc::Member<Window> view_;

  public: UiEventInit();
  public: virtual ~UiEventInit();

  public: int detail() const { return detail_; }
  public: void set_detail(int detail) { detail_ = detail; }
  public: Window* view() const { return view_.get(); }
  public: void set_view(Window* view) { view_ = view; }

  // dom::Dictionary
  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_ui_event_init_h)
