// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_
#define EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_

#include "evita/dom/forms/form_control.h"

#include "evita/gc/member.h"

namespace dom {

class TextFieldSelection;

namespace bindings {
class TextFieldControlClass;
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
class TextFieldControl final
    : public v8_glue::Scriptable<TextFieldControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldControl);

 public:
  ~TextFieldControl() final;

  size_t length() const { return value_.length(); }
  TextFieldSelection* selection() const { return selection_.get(); }
  const base::string16& value() const { return value_; }

  void DidChangeSelection();

 private:
  friend class bindings::TextFieldControlClass;

  // bindings
  TextFieldControl();

  float scroll_left() const { return scroll_left_; }
  void set_scroll_left(float left);

  // API: Set |value| property to the first line, excluding newline character,
  // of |new_value|.
  void set_value(const base::string16& new_value);

  int MapPointToOffset(int x, int y) const;

  // FormControl
  std::unique_ptr<domapi::FormControl> Paint(
      const FormPaintInfo& paint_info) const final;

  float scroll_left_ = 0;
  gc::Member<TextFieldSelection> selection_;
  base::string16 value_;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_TEXT_FIELD_CONTROL_H_
