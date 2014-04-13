// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/text_window.h"

#include "base/bind.h"
#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/windows/point.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/text/range.h"
#include "evita/dom/script_host.h"
#include "evita/dom/windows/text_selection.h"
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
        .SetProperty("range_", &TextWindow::view_range)
        .SetMethod("compute_", &TextWindow::ComputeMotion)
        .SetMethod("makeSelectionVisible", &TextWindow::MakeSelectionVisible)
        .SetMethod("mapPointToPosition_", &TextWindow::MapPointToPosition)
        .SetMethod("mapPositionToPoint_", &TextWindow::MapPositionToPoint)
        .SetMethod("reconvert_", &TextWindow::Reconvert)
        .SetMethod("scroll", &TextWindow::Scroll);
  }

  DISALLOW_COPY_AND_ASSIGN(TextWindowWrapperInfo);
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
  ScriptHost::instance()->view_delegate()->CreateTextWindow(this);
}

TextWindow::~TextWindow() {
}

::Selection* TextWindow::view_selection() const {
  return static_cast<TextSelection*>(selection())->view_selection();
}

text::Posn TextWindow::ComputeMotion(int method,
    v8_glue::Optional<text::Posn> opt_position,
    v8_glue::Optional<int> opt_count,
    v8_glue::Optional<domapi::FloatPoint> opt_point) {
  TextWindowCompute data;
  data.method = static_cast<TextWindowCompute::Method>(method);
  data.count = opt_count.get(1);
  data.position = opt_position.get(0);
  data.x = opt_point.is_supplied ? opt_point.value.x() : 0.0f;
  data.y = opt_point.is_supplied ? opt_point.value.y() : 0.0f;
  return ScriptHost::instance()->view_delegate()->ComputeOnTextWindow(
      id(), data);
}

void TextWindow::MakeSelectionVisible() {
  ScriptHost::instance()->view_delegate()->MakeSelectionVisible(
      window_id());
}

text::Posn TextWindow::MapPointToPosition(float x, float y) {
  return ScriptHost::instance()->view_delegate()->MapPointToPosition(
    id(), x, y);
}

domapi::FloatPoint TextWindow::MapPositionToPoint(text::Posn position) {
  return ScriptHost::instance()->view_delegate()->
      MapPositionToPoint(id(), position);
}

void TextWindow::Reconvert(text::Posn start, text::Posn end) {
  ScriptHost::instance()->view_delegate()->
      Reconvert(window_id(), start, end);
}

void TextWindow::Scroll(int direction) {
  ScriptHost::instance()->view_delegate()->
      ScrollTextWindow(window_id(), direction);
}

}  // namespace dom
