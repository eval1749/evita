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
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TextWindowWrapperInfo
//
class TextWindowWrapperInfo :
    public v8_glue::DerivedWrapperInfo<TextWindow, DocumentWindow> {

  public: TextWindowWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~TextWindowWrapperInfo() = default;

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
    builder
        .SetMethod("makeSelectionVisible", &TextWindow::MakeSelectionVisible);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
DEFINE_SCRIPTABLE_OBJECT(TextWindow, TextWindowWrapperInfo);

TextWindow::TextWindow(Range* selection_range)
    : ScriptableBase(new Selection(this, selection_range)),
      view_range_(new Range(selection_range->document(), 0, 0)) {
  ScriptController::instance()->view_delegate()->CreateTextWindow(this);
}

TextWindow::~TextWindow() {
}

void TextWindow::MakeSelectionVisible() {
  ScriptController::instance()->view_delegate()->MakeSelectionVisible(
      window_id());
}

}  // namespace dom
