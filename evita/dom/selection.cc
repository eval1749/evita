// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/selection.h"

#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/text_window.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"
#include "evita/vi_Selection.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// SelectionWrapperInfo
//
class SelectionWrapperInfo : public v8_glue::WrapperInfo {
  public: SelectionWrapperInfo() : v8_glue::WrapperInfo("Selection") {
  }
  public: ~SelectionWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &SelectionWrapperInfo::NewSelection);
  }

  private: static Selection* NewSelection() {
    ScriptController::instance()->ThrowError("Can't create selection.");
    return nullptr;
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("active", &Selection::active)
        .SetProperty("document", &Selection::document)
        .SetProperty("range", &Selection::range)
        .SetProperty("startIsActive", &Selection::start_is_active,
                     &Selection::set_start_is_active)
        .SetProperty("window", &Selection::window);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(TextWindow* text_window, Range* range)
    : document_(range->document()), text_window_(text_window),
      view_selection_(::Selection::Create(*range->text_range())),
      range_(new Range(document_, view_selection_)) {
}

Selection::~Selection() {
}

Posn Selection::active() const {
  return start_is_active() ? range_->start() : range_->end();
}

bool Selection::start_is_active() const {
  return view_selection_->IsStartActive();
}

void Selection::set_start_is_active(bool start_is_active) {
  view_selection_->SetStartIsActive(start_is_active);
}

v8_glue::WrapperInfo* Selection::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(SelectionWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
