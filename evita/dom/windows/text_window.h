// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_text_window_h)
#define INCLUDE_evita_dom_text_window_h

#include "evita/dom/windows/document_window.h"

#include "evita/dom/windows/rect.h"

class Selection;

namespace domapi {
class FloatPoint;
class FloatRect;
}

namespace dom {
class Document;
class Range;
class Selection;

namespace bindings {
class TextWindowClass;
}

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class TextWindow : public v8_glue::Scriptable<TextWindow, DocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TextWindow);
  friend class bindings::TextWindowClass;

  private: float zoom_;

  private: explicit TextWindow(Range* selection_range);
  public: virtual ~TextWindow();

  public: ::Selection* view_selection() const;
  private: float zoom() const { return zoom_; }
  private: void set_zoom(float new_zoom);

  private: text::Posn ComputeMotion(int method);
  private: text::Posn ComputeMotion(int method, text::Posn position);
  private: text::Posn ComputeMotion(int method, text::Posn position,
                                    int count);
  private: text::Posn ComputeMotion(int method, text::Posn position,
                                    int count, const domapi::FloatPoint& point);
  private: domapi::FloatRect HitTestTextPosition(text::Posn position);
  private: void MakeSelectionVisible();
  private: TextWindow* NewTextWindow(Range* range);
  private: text::Posn MapPointToPosition(float x, float y);
  private: void Reconvert(const base::string16& text);
  private: void Scroll(int direction);

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_text_window_h)
