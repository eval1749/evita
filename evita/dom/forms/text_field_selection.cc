// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/text_field_selection.h"

#include "base/strings/stringprintf.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/script_host.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// TextFieldSelectionClass
//
class TextFieldSelectionClass : public v8_glue::WrapperInfo {
  public: TextFieldSelectionClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~TextFieldSelectionClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &TextFieldSelectionClass::NewTextFieldSelection);
  }

  private: static TextFieldSelection* NewTextFieldSelection() {
    ScriptHost::instance()->ThrowError("Can't create TextFieldSelection.");
    return nullptr;
  }

  // v8_glue::WrapperClassInfo
  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("control", &TextFieldSelection::control)
        .SetProperty("end", &TextFieldSelection::end,
            &TextFieldSelection::set_end)
        .SetProperty("start", &TextFieldSelection::start,
            &TextFieldSelection::set_start)
        .SetProperty("startIsActive", &TextFieldSelection::start_is_active,
            &TextFieldSelection::set_start_is_active)
        .SetMethod("setRange", &TextFieldSelection::SetRange);
  }

  DISALLOW_COPY_AND_ASSIGN(TextFieldSelectionClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldSelection
//
DEFINE_SCRIPTABLE_OBJECT(TextFieldSelection, TextFieldSelectionClass);

TextFieldSelection::TextFieldSelection(TextFieldControl* control)
    : ScriptableBase(), control_(control), end_(0), start_(0),
      start_is_active_(false) {
}

TextFieldSelection::~TextFieldSelection() {
}

void TextFieldSelection::set_end(int new_end) {
  if (end_ == new_end || !ValidateOffset(new_end))
    return;
  end_ = new_end;
  start_ = std::min(start_, end_);
  control_->DidChangeSelection();
}

void TextFieldSelection::set_start(int new_start) {
  if (start_ == new_start || !ValidateOffset(new_start))
    return;
  start_ = new_start;
  end_ = std::max(end_, start_);
  control_->DidChangeSelection();
}

void TextFieldSelection::set_start_is_active(bool new_start_is_active) {
  if (start_is_active_ == new_start_is_active)
    return;
  start_is_active_ = new_start_is_active;
  control_->DidChangeSelection();
}

void TextFieldSelection::DidChangeValue() {
  int max_offset = static_cast<int>(control_->value().size());
  end_ = std::min(end_, max_offset);
  start_ = std::min(start_, max_offset);
}

void TextFieldSelection::SetRange(int new_start, int new_end,
    v8_glue::Optional<bool> opt_start_is_active) {
  if (!ValidateOffset(new_start) || !ValidateOffset(new_end))
    return;
  if (new_start > new_end) {
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "Start(%d) must be less than or equal to end(%d)",
        new_start, new_end));
    return;
  }

  auto const new_start_is_active = opt_start_is_active.get(false);
  if (end_ == new_end && start_ == new_start &&
      start_is_active_ == new_start_is_active) {
    return;
  }

  end_ = new_end;
  start_ = new_start;
  start_is_active_ = new_start_is_active;
  control_->DidChangeSelection();
}

bool TextFieldSelection::ValidateOffset(int offset){
  if (offset < 0 || offset > static_cast<int>(control_->value().size())) {
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "Bad text field offset %d.", offset));
    return false;
  }
  return true;
}

}  // namespace dom
