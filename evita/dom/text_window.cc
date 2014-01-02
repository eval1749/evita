// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/text_window.h"

#include "base/bind.h"
#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/selection.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TextWindowWrapperInfo
//
class TextWindowWrapperInfo : public v8_glue::WrapperInfo {
  public: TextWindowWrapperInfo() : v8_glue::WrapperInfo("TextWindow") {
  }
  public: ~TextWindowWrapperInfo() = default;

  private: virtual WrapperInfo* inherit_from() const override {
    return Window::static_wrapper_info();
  }

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TextWindowWrapperInfo::NewTextWindow);
  }

  private: static TextWindow* NewTextWindow(Range* selection_range) {
    return new TextWindow(selection_range);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetProperty("document", &TextWindow::document);
    builder.SetProperty("selection", &TextWindow::selection);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(Range* selection_range)
    : selection_(new Selection(this, selection_range)),
      view_range_(new Range(selection_range->document(), 0, 0)) {
  ScriptController::instance()->view_delegate()->CreateTextWindow(this);
}

TextWindow::~TextWindow() {
}

Document* TextWindow::document() const {
  return selection_->document();
}

v8_glue::WrapperInfo* TextWindow::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(TextWindowWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
