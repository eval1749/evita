// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_text_control_h)
#define INCLUDE_evita_dom_forms_text_control_h

#include "evita/dom/forms/form_control.h"

#include "evita/gc/member.h"

namespace dom {

class TextFieldSelection;

class TextFieldControl
    : public v8_glue::Scriptable<TextFieldControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldControl);

  private: base::string16 value_;
  private: gc::Member<TextFieldSelection> selection_;

  public: TextFieldControl();
  public: virtual ~TextFieldControl();

  public: size_t length() const { return value_.length(); }
  public: TextFieldSelection* selection() const { return selection_.get(); }
  public: const base::string16& value() const { return value_; }

  // API: Set |value| property to the first line, excluding newline character,
  // of |new_value|.
  public: void set_value(const base::string16& new_value);

  public: void DidChangeSelection();

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_text_control_h)
