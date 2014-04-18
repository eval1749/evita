// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_text_window_h)
#define INCLUDE_evita_dom_text_window_h

#include "evita/dom/windows/document_window.h"

#include "evita/dom/windows/rect.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/either.h"
#include "evita/v8_glue/optional.h"

class Selection;

namespace dom {
class Document;
class Range;
class Selection;

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class TextWindow : public v8_glue::Scriptable<TextWindow, DocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TextWindow);

  public: explicit TextWindow(Range* selection_range);
  public: virtual ~TextWindow();

  public: ::Selection* view_selection() const;

  public: text::Posn ComputeMotion(int method,
      v8_glue::Optional<text::Posn> opt_position,
      v8_glue::Optional<int> opt_count,
      v8_glue::Optional<domapi::FloatPoint> opt_point);
  public: domapi::FloatRect HitTestTextPosition(text::Posn position);
  public: void MakeSelectionVisible();
  public: text::Posn MapPointToPosition(float x, float y);
  public: void Reconvert(text::Posn start, text::Posn end);
  public: void Scroll(int direction);

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_text_window_h)
