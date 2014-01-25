// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_text_window_h)
#define INCLUDE_evita_dom_text_window_h

#include "evita/dom/document_window.h"

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

  private: gc::Member<Range> view_range_;

  public: TextWindow(Range* selection_range);
  public: virtual ~TextWindow();

  public: Range* view_range() const { return view_range_; }
  public: ::Selection* view_selection() const;

  public: v8::Handle<v8::Value> Compute(int method,
      v8_glue::Optional<v8_glue::Either<int, float>> opt_either1,
      v8_glue::Optional<v8_glue::Either<int, float>> opt_either2);
  public: void MakeSelectionVisible();

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_text_window_h)
