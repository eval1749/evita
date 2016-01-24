// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/text_window.h"

#include "evita/bindings/v8_glue_TextDocumentEventInit.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/events/text_document_event.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/text/text_range.h"
#include "evita/dom/script_host.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/windows/text_selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(TextRange* selection_range)
    : selection_(new TextSelection(this, selection_range)), zoom_(1.0f) {
  ScriptHost::instance()->view_delegate()->CreateTextWindow(
      window_id(), static_cast<TextSelection*>(selection())->text_selection());
}

TextWindow::~TextWindow() {}

TextDocument* TextWindow::document() const {
  return selection_->document();
}

void TextWindow::set_zoom(float new_zoom, ExceptionState* exception_state) {
  if (zoom_ == new_zoom)
    return;
  if (new_zoom <= 0.0f) {
    exception_state->ThrowRangeError(
        "TextWindow zoom must be greater than zero.");
    return;
  }
  zoom_ = new_zoom;
  ScriptHost::instance()->view_delegate()->SetTextWindowZoom(window_id(),
                                                             zoom_);
}

text::Offset TextWindow::ComputeMotion(int method,
                                       text::Offset position,
                                       int count,
                                       const domapi::FloatPoint& point) {
  domapi::TextWindowCompute data;
  data.method = static_cast<domapi::TextWindowCompute::Method>(method);
  data.count = count;
  data.position = position;
  data.x = point.x();
  data.y = point.y();
  return ScriptHost::instance()->view_delegate()->ComputeOnTextWindow(
      window_id(), data);
}

text::Offset TextWindow::ComputeMotion(int method,
                                       text::Offset position,
                                       int count) {
  return ComputeMotion(method, position, count, domapi::FloatPoint());
}

text::Offset TextWindow::ComputeMotion(int method, text::Offset position) {
  return ComputeMotion(method, position, 1, domapi::FloatPoint());
}

text::Offset TextWindow::ComputeMotion(int method) {
  return ComputeMotion(method, text::Offset(0), 1, domapi::FloatPoint());
}

domapi::FloatRect TextWindow::HitTestTextPosition(text::Offset position) {
  return ScriptHost::instance()->view_delegate()->HitTestTextPosition(
      window_id(), position);
}

void TextWindow::MakeSelectionVisible() {
  ScriptHost::instance()->view_delegate()->MakeSelectionVisible(window_id());
}

v8::Local<v8::Promise> TextWindow::HitTestPoint(float x, float y) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::ViewDelegate::MapTextWindowPointToOffset,
                 base::Unretained(ScriptHost::instance()->view_delegate()),
                 window_id(), x, y));
}

// static
TextWindow* TextWindow::NewTextWindow(TextRange* selection_range) {
  return new TextWindow(selection_range);
}

void TextWindow::Reconvert(const base::string16& text) {
  ScriptHost::instance()->view_delegate()->Reconvert(window_id(), text);
}

void TextWindow::Scroll(int direction) {
  ScriptHost::instance()->view_delegate()->ScrollTextWindow(window_id(),
                                                            direction);
}

// Window
void TextWindow::DidDestroyWindow() {
  // TODO(eval1749): We should dispatch "detach" event in JavaScript rather than
  // in C++;
  Window::DidDestroyWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"detach", init));
}

void TextWindow::DidRealizeWindow() {
  // TODO(eval1749): We should dispatch "attach" event in JavaScript rather than
  // in C++;
  Window::DidRealizeWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"attach", init));
}

}  // namespace dom
