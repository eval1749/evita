// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
#define EVITA_DOM_WINDOWS_TEXT_WINDOW_H_

#include "evita/dom/windows/text_document_window.h"

#include "evita/dom/windows/rect.h"

namespace domapi {
class FloatPoint;
class FloatRect;
}

namespace text {
class Selection;
}

namespace dom {
class TextDocument;
class Range;

namespace bindings {
class TextWindowClass;
}

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextWindow.
class TextWindow final
    : public v8_glue::Scriptable<TextWindow, TextDocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TextWindow);

 public:
  ~TextWindow() final;

 private:
  friend class bindings::TextWindowClass;

  explicit TextWindow(Range* selection_range);

  float zoom() const { return zoom_; }
  void set_zoom(float new_zoom);

  text::Offset ComputeMotion(int method);
  text::Offset ComputeMotion(int method, text::Offset position);
  text::Offset ComputeMotion(int method, text::Offset position, int count);
  text::Offset ComputeMotion(int method,
                             text::Offset position,
                             int count,
                             const domapi::FloatPoint& point);
  v8::Handle<v8::Promise> HitTestPoint(float x, float y);
  domapi::FloatRect HitTestTextPosition(text::Offset position);
  void MakeSelectionVisible();
  TextWindow* NewTextWindow(Range* range);
  void Reconvert(const base::string16& text);
  void Scroll(int direction);

  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
