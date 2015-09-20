// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_
#define EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_

#include "evita/dom/forms/form_control.h"

#include "evita/gc/member.h"

namespace dom {

class TextFieldSelection;

namespace bindings {
class TextFieldControlClass;
}

class TextFieldControl final
    : public v8_glue::Scriptable<TextFieldControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldControl);

 public:
  ~TextFieldControl() final;

  size_t length() const { return value_.length(); }
  TextFieldSelection* selection() const { return selection_.get(); }
  const base::string16& value() const { return value_; }

  void DidChangeSelection();

  int MapPointToOffset(float x, float y) const;

 private:
  friend class bindings::TextFieldControlClass;

  TextFieldControl();

  // API: Set |value| property to the first line, excluding newline character,
  // of |new_value|.
  void set_value(const base::string16& new_value);

  base::string16 value_;
  gc::Member<TextFieldSelection> selection_;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_
