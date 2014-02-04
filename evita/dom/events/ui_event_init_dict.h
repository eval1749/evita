// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_ui_event_init_dict_h)
#define INCLUDE_evita_dom_events_ui_event_init_dict_h

#include "evita/dom/events/event_init_dict.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class UiEventInitDict : public EventInitDict {
  private: int detail_;
  private: gc::Member<Window> view_;

  public: UiEventInitDict();
  public: virtual ~UiEventInitDict();

  public: int detail() const { return detail_; }
  public: Window* view() const { return view_.get(); }

  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::String> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(UiEventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_ui_event_init_dict_h)
