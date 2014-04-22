// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_form_event_init_h)
#define INCLUDE_evita_dom_events_form_event_init_h

#include "evita/bindings/EventInit.h"

namespace domapi {
struct FormEvent;
}

namespace dom {

class FormEventInit : public EventInit {
  private: base::string16 data_;

  public: explicit FormEventInit(const domapi::FormEvent& event);
  public: FormEventInit();
  public: virtual ~FormEventInit();

  public: const base::string16& data() const { return data_; }
  public: void set_data(const base::string16& data) { data_ = data; }

  // dom::Dictionary
  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_form_event_init_h)
