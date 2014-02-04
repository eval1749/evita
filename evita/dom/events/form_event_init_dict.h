// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_form_event_init_dict_h)
#define INCLUDE_evita_dom_events_form_event_init_dict_h

#include "evita/dom/events/event_init_dict.h"

namespace dom {

class FormEventInitDict : public EventInitDict {
  private: base::string16 data_;

  public: FormEventInitDict();
  public: virtual ~FormEventInitDict();

  public: const base::string16& data() const { return data_; }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::String> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(FormEventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_form_event_init_dict_h)
