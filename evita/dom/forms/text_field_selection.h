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
  private: int end_;
  private: int start_;
  private: bool start_is_active_;

  public: TextFieldSelection(TextFieldControl* control);
  public: virtual ~TextFieldSelection();

  public: TextFieldControl* control() const { return control_.get(); }
  public: int end() const { return end_; }
  public: void set_end(int new_end);
  public: int start() const { return start_; }
  public: void set_start(int new_start);
  public: bool start_is_active() const { return start_is_active_; }
  public: void set_start_is_active(bool new_start_is_active);

  // Called when value of text field control changed.
  public: void DidChangeValue();

  // API for changing range of selection.
  public: void SetRange(int start, int end,
                        v8_glue::Optional<bool> opt_start_is_actie);

  private: bool ValidateOffset(int offset);

  DISALLOW_COPY_AND_ASSIGN(TextFieldSelection);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_text_field_selection_h)
