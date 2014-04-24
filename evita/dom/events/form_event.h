// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_from_event_h)
#define INCLUDE_evita_dom_events_from_event_h

#include "base/strings/string16.h"
#include "evita/dom/events/event.h"
#include "evita/v8_glue/nullable.h"

namespace domapi {
struct FormEvent;
}

namespace dom {

class FormEventInit;
class Window;

namespace bindings {
class FormEventClass;
}

class FormEvent : public v8_glue::Scriptable<FormEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(FormEvent)
  friend class bindings::FormEventClass;

  private: base::string16 data_;

  public: explicit FormEvent(const domapi::FormEvent& event);
  private: FormEvent(const base::string16& type,
                     const FormEventInit& init_dict);
  private: FormEvent(const base::string16& type);
  public: virtual ~FormEvent();

  // Expose for |CheckBoxControl|.
  public: const base::string16& data() const { return data_; }

  DISALLOW_COPY_AND_ASSIGN(FormEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_from_event_h)
