// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_text_window_h)
#define INCLUDE_evita_dom_text_window_h

#include "evita/dom/window.h"

#include "evita/gc/member.h"

namespace dom {
class Document;
class Range;
class Selection;

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class TextWindow : public v8_glue::Scriptable<TextWindow, Window> {
  private: gc::Member<Selection> selection_;
  private: gc::Member<Range> view_range_;

  public: TextWindow(Range* selection_range);
  public: virtual ~TextWindow();

  public: Document* document() const;
  public: Selection* selection() const { return selection_; }
  public: static v8_glue::WrapperInfo* static_wrapper_info();
  public: Range* view_range() const { return view_range_; }

  public: void MakeSelectionVisible();

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_text_window_h)
