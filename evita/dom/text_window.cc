// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/text_window.h"

#include "base/bind.h"
#include "evita/dom/converter.h"
#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/text_selection.h"
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
        .SetMethod("compute_", &TextWindow::Compute)
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
    : ScriptableBase(new TextSelection(this, selection_range)),
      view_range_(new Range(selection_range->document(), 0, 0)) {
  ScriptController::instance()->view_delegate()->CreateTextWindow(this);
}

TextWindow::~TextWindow() {
}

::Selection* TextWindow::view_selection() const {
  return static_cast<TextSelection*>(selection())->view_selection();
}

v8::Handle<v8::Value> TextWindow::Compute(int method,
      v8_glue::Optional<v8_glue::Either<int, float>> opt_either1,
      v8_glue::Optional<v8_glue::Either<int, float>> opt_either2) {
  TextWindowCompute data;
  data.method = static_cast<TextWindowCompute::Method>(method);
  data.int1 = opt_either1.is_supplied && opt_either1.value.is_left ?
      opt_either1.value.left : 0;
  data.int2 = opt_either2.is_supplied && opt_either2.value.is_left ?
      opt_either2.value.left : 0;
  data.float1 = opt_either1.is_supplied && !opt_either1.value.is_left ?
      opt_either1.value.right : 0.0f;
  data.float2 = opt_either2.is_supplied && !opt_either2.value.is_left ?
      opt_either2.value.right : 0.0f;
  ScriptController::instance()->view_delegate()->ComputeOnTextWindow(
    id(), &data, nullptr);
  switch (data.method) {
    case TextWindowCompute::Method::EndOfWindow:
    case TextWindowCompute::Method::EndOfWindowLine:
    case TextWindowCompute::Method::MapPointToPosition:
    case TextWindowCompute::Method::MoveWindow:
    case TextWindowCompute::Method::MoveWindowLine:
    case TextWindowCompute::Method::StartOfWindow:
    case TextWindowCompute::Method::StartOfWindowLine:
      return gin::ConvertToV8(v8::Isolate::GetCurrent(), data.int1);
    case TextWindowCompute::Method::MapPositionToPoint:
      return gin::ConvertToV8(v8::Isolate::GetCurrent(), data.float1);
  }
  ScriptController::instance()->ThrowError("Invalid method.");
  return v8::Undefined(v8::Isolate::GetCurrent());
}

void TextWindow::MakeSelectionVisible() {
  ScriptController::instance()->view_delegate()->MakeSelectionVisible(
      window_id());
}

}  // namespace dom
