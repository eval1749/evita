// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
#define EVITA_DOM_WINDOWS_TEXT_WINDOW_H_

#include "evita/dom/windows/document_window.h"

#include "evita/dom/windows/rect.h"

namespace domapi {
class FloatPoint;
class FloatRect;
}

namespace text {
class Selection;
}

namespace dom {
class Document;
class Range;

namespace bindings {
class TextWindowClass;
}

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextWindow.
class TextWindow final
    : public v8_glue::Scriptable<TextWindow, DocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TextWindow);

 public:
  ~TextWindow() final;

 private:
  friend class bindings::TextWindowClass;

  explicit TextWindow(Range* selection_range);

  float zoom() const { return zoom_; }
  void set_zoom(float new_zoom);

  text::Posn ComputeMotion(int method);
  text::Posn ComputeMotion(int method, text::Posn position);
  text::Posn ComputeMotion(int method, text::Posn position, int count);
  text::Posn ComputeMotion(int method,
                           text::Posn position,
                           int count,
                           const domapi::FloatPoint& point);
  domapi::FloatRect HitTestTextPosition(text::Posn position);
  void MakeSelectionVisible();
  TextWindow* NewTextWindow(Range* range);
  text::Posn MapPointToPosition(float x, float y);
  void Reconvert(const base::string16& text);
  void Scroll(int direction);

  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
