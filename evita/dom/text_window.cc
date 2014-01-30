// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/text_window.h"

#include "base/bind.h"
#include "evita/dom/converter.h"
#include "evita/dom/document.h"
#include "evita/dom/point.h"
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
        .SetMethod("compute_", &TextWindow::ComputeMotion)
        .SetMethod("makeSelectionVisible", &TextWindow::MakeSelectionVisible)
        .SetMethod("mapPointToPosition_", &TextWindow::MapPointToPosition)
        .SetMethod("mapPositionToPoint_", &TextWindow::MapPositionToPoint);
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

text::Posn TextWindow::ComputeMotion(int method,
    v8_glue::Optional<text::Posn> opt_position,
    v8_glue::Optional<int> opt_count,
    v8_glue::Optional<Point*> opt_point) {
  TextWindowCompute data;
  data.method = static_cast<TextWindowCompute::Method>(method);
  data.count = opt_count.get(1);
  data.position = opt_position.get(0);
  data.x = opt_point.is_supplied ? opt_point.value->x() : 0.0f;
  data.y = opt_point.is_supplied ? opt_point.value->y() : 0.0f;
  ScriptController::instance()->view_delegate()->ComputeOnTextWindow(
    id(), &data, nullptr);
  switch (data.method) {
    case TextWindowCompute::Method::EndOfWindow:
    case TextWindowCompute::Method::EndOfWindowLine:
    case TextWindowCompute::Method::MapPointToPosition:
    case TextWindowCompute::Method::MoveScreen:
    case TextWindowCompute::Method::MoveWindow:
    case TextWindowCompute::Method::MoveWindowLine:
    case TextWindowCompute::Method::StartOfWindow:
    case TextWindowCompute::Method::StartOfWindowLine:
      return data.position;
  }
  ScriptController::instance()->ThrowError("Invalid method.");
  return 0;
}

void TextWindow::MakeSelectionVisible() {
  ScriptController::instance()->view_delegate()->MakeSelectionVisible(
      window_id());
}

text::Posn TextWindow::MapPointToPosition(float x, float y) {
  TextWindowCompute data;
  data.method = TextWindowCompute::Method::MapPointToPosition;
  data.x = x;
  data.y = y;
  ScriptController::instance()->view_delegate()->ComputeOnTextWindow(
    id(), &data, nullptr);
  return data.position;
}

Point* TextWindow::MapPositionToPoint(text::Posn position) {
  TextWindowCompute data;
  data.method = TextWindowCompute::Method::MapPositionToPoint;
  data.position = position;
  ScriptController::instance()->view_delegate()->ComputeOnTextWindow(
    id(), &data, nullptr);
  return new Point(data.x, data.y);
}

}  // namespace dom
