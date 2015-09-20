// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_FORM_EVENT_H_
#define EVITA_DOM_EVENTS_FORM_EVENT_H_

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

class FormEvent final : public v8_glue::Scriptable<FormEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(FormEvent)

 public:
  explicit FormEvent(const domapi::FormEvent& event);
  ~FormEvent() final;

  // Expose for |CheckBoxControl|.
  const base::string16& data() const { return data_; }

 private:
  friend class bindings::FormEventClass;

  FormEvent(const base::string16& type, const FormEventInit& init_dict);
  explicit FormEvent(const base::string16& type);

  base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(FormEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_FORM_EVENT_H_
