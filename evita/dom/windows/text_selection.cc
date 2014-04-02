// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/text_selection.h"

#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/script_host.h"
#include "evita/dom/windows/text_window.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"
#include "evita/vi_Selection.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TextSelectionClass
//
class TextSelectionClass :
    public v8_glue::DerivedWrapperInfo<TextSelection, Selection> {

  public: TextSelectionClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TextSelectionClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &TextSelectionClass::NewTextSelection);
  }

  private: static TextSelection* NewTextSelection() {
    ScriptHost::instance()->ThrowError("Can't create selection.");
    return nullptr;
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("anchorOffset", &TextSelection::anchor_offset)
        .SetProperty("focusOffset", &TextSelection::focus_offset)
        .SetProperty("range", &TextSelection::range)
        .SetProperty("startIsActive", &TextSelection::start_is_active,
                     &TextSelection::set_start_is_active);
  }
  DISALLOW_COPY_AND_ASSIGN(TextSelectionClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
DEFINE_SCRIPTABLE_OBJECT(TextSelection, TextSelectionClass);

TextSelection::TextSelection(TextWindow* text_window, Range* range)
    : ScriptableBase(text_window, range->document()),
      range_(new Range(range->document(), range->start(), range->end())),
      view_selection_(new ::Selection(range_->text_range())) {
}

TextSelection::~TextSelection() {
}

Posn TextSelection::anchor_offset() const {
  return start_is_active() ? range_->end() : range_->start();
}

Posn TextSelection::focus_offset() const {
  return start_is_active() ? range_->start() : range_->end();
}

bool TextSelection::start_is_active() const {
  return view_selection_->IsStartActive();
}

void TextSelection::set_start_is_active(bool start_is_active) {
  view_selection_->SetStartIsActive(start_is_active);
}

}  // namespace dom
