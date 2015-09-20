// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_TEXT_FIELD_SELECTION_H_
#define EVITA_DOM_FORMS_TEXT_FIELD_SELECTION_H_

#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class TextFieldControl;

namespace bindings {
class TextFieldSelectionClass;
}

class TextFieldSelection final
    : public v8_glue::Scriptable<TextFieldSelection> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldSelection);

 public:
  explicit TextFieldSelection(TextFieldControl* control);
  ~TextFieldSelection() final;

  int anchor_offset() const { return anchor_offset_; }
  int focus_offset() const { return focus_offset_; }

  // Called when value of text field control changed.
  void DidChangeValue();

 private:
  friend class bindings::TextFieldSelectionClass;

  bool collapsed() const { return anchor_offset_ == focus_offset_; }
  TextFieldControl* control() const { return control_.get(); }
  void set_anchor_offset(int new_anchor_offset);
  void set_focus_offset(int new_focus_offset);

  int NormalizeOffset(int offset) const;

  gc::Member<TextFieldControl> control_;
  int anchor_offset_;
  int focus_offset_;

  DISALLOW_COPY_AND_ASSIGN(TextFieldSelection);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_TEXT_FIELD_SELECTION_H_
