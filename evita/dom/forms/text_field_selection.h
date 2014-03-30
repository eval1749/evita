// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_text_field_selection_h)
#define INCLUDE_evita_dom_forms_text_field_selection_h

#include "evita/gc/member.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class TextFieldControl;

class TextFieldSelection : public v8_glue::Scriptable<TextFieldSelection> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldSelection);

  private: gc::Member<TextFieldControl> control_;
  private: int anchor_offset_;
  private: int focus_offset_;

  public: TextFieldSelection(TextFieldControl* control);
  public: virtual ~TextFieldSelection();

  public: bool collapsed() const { return anchor_offset_ == focus_offset_; }
  public: TextFieldControl* control() const { return control_.get(); }
  public: int anchor_offset() const { return anchor_offset_; }
  public: void set_anchor_offset(int new_anchor_offset);
  public: int focus_offset() const { return focus_offset_; }
  public: void set_focus_offset(int new_focus_offset);

  // Called when value of text field control changed.
  public: void DidChangeValue();

  private: int NormalizeOffset(int offset) const;

  DISALLOW_COPY_AND_ASSIGN(TextFieldSelection);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_text_field_selection_h)
