// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/text_window.h"

#include "base/bind.h"
#include "evita/dom/text/document.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/text/range.h"
#include "evita/dom/script_host.h"
#include "evita/dom/windows/text_selection.h"
#include "evita/dom/view_delegate.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(Range* selection_range)
    : ScriptableBase(new TextSelection(this, selection_range)), zoom_(1.0f) {
  ScriptHost::instance()->view_delegate()->CreateTextWindow(this);
}

TextWindow::~TextWindow() {
}

text::Selection* TextWindow::text_selection() const {
  return static_cast<TextSelection*>(selection())->text_selection();
}

void TextWindow::set_zoom(float new_zoom) {
  if (zoom_ == new_zoom)
    return;
  if (new_zoom <= 0.0f) {
    ScriptHost::instance()->ThrowRangeError(
        "TextWindow zoom must be greater than zero.");
    return;
  }
  zoom_ = new_zoom;
  ScriptHost::instance()->view_delegate()->SetTextWindowZoom(
      window_id(), zoom_);
}

text::Posn TextWindow::ComputeMotion(int method, text::Posn position,
                                     int count, const domapi::FloatPoint& point) {
  TextWindowCompute data;
  data.method = static_cast<TextWindowCompute::Method>(method);
  data.count = count;
  data.position = position;
  data.x = point.x();
  data.y = point.y();
  return ScriptHost::instance()->view_delegate()->ComputeOnTextWindow(
      window_id(), data);
}

text::Posn TextWindow::ComputeMotion(int method, text::Posn position,
                                     int count) {
  return ComputeMotion(method, position, count, domapi::FloatPoint());
}

text::Posn TextWindow::ComputeMotion(int method, text::Posn position) {
  return ComputeMotion(method, position, 1, domapi::FloatPoint());
}

text::Posn TextWindow::ComputeMotion(int method) {
  return ComputeMotion(method, 0, 1, domapi::FloatPoint());
}

domapi::FloatRect TextWindow::HitTestTextPosition(text::Posn position) {
  return ScriptHost::instance()->view_delegate()->
      HitTestTextPosition(window_id(), position);
}

void TextWindow::MakeSelectionVisible() {
  ScriptHost::instance()->view_delegate()->MakeSelectionVisible(
      window_id());
}

text::Posn TextWindow::MapPointToPosition(float x, float y) {
  return ScriptHost::instance()->view_delegate()->MapPointToPosition(
    window_id(), x, y);
}

// static
TextWindow* TextWindow::NewTextWindow(Range* selection_range) {
  return new TextWindow(selection_range);
}

void TextWindow::Reconvert(const base::string16& text) {
  ScriptHost::instance()->view_delegate()->Reconvert(window_id(), text);
}

void TextWindow::Scroll(int direction) {
  ScriptHost::instance()->view_delegate()->
      ScrollTextWindow(window_id(), direction);
}

}  // namespace dom
