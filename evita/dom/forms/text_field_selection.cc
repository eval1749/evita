// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/text_field_selection.h"

#include <algorithm>

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
        .SetProperty("collapsed", &TextFieldSelection::collapsed)
        .SetProperty("control", &TextFieldSelection::control)
        .SetProperty("anchorOffset", &TextFieldSelection::anchor_offset,
            &TextFieldSelection::set_anchor_offset)
        .SetProperty("end", &TextFieldSelection::end)
        .SetProperty("focusOffset", &TextFieldSelection::focus_offset,
            &TextFieldSelection::set_focus_offset)
        .SetProperty("start", &TextFieldSelection::start);
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
    : ScriptableBase(), control_(control), anchor_offset_(0),
      focus_offset_(0) {
}

TextFieldSelection::~TextFieldSelection() {
}

void TextFieldSelection::set_anchor_offset(int anchor_offset) {
  auto const new_anchor_offset = NormalizeOffset(anchor_offset);
  if (anchor_offset_ == new_anchor_offset)
    return;
  anchor_offset_ = new_anchor_offset;
  control_->DidChangeSelection();
}

int TextFieldSelection::end() const {
  return std::max(anchor_offset_, focus_offset_);
}

void TextFieldSelection::set_focus_offset(int focus_offset) {
  auto const new_focus_offset = NormalizeOffset(focus_offset);
  if (focus_offset_ == new_focus_offset)
    return;
  focus_offset_ = new_focus_offset;
  control_->DidChangeSelection();
}

int TextFieldSelection::start() const {
  return std::min(anchor_offset_, focus_offset_);
}

void TextFieldSelection::DidChangeValue() {
  anchor_offset_ = NormalizeOffset(anchor_offset_);
  focus_offset_ = NormalizeOffset(focus_offset_);
}

int TextFieldSelection::NormalizeOffset(int offset) const {
  return std::max(0, std::min(offset, static_cast<int>(control_->length())));
}

}  // namespace dom
