// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_text_selection_h)
#define INCLUDE_evita_dom_text_selection_h

#include "evita/dom/windows/selection.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

class Selection;

namespace dom {

class Document;
class Range;
class TextWindow;

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
class TextSelection : public v8_glue::Scriptable<TextSelection, Selection> {
  DECLARE_SCRIPTABLE_OBJECT(TextSelection);

  private: gc::Member<Range> range_;
  private: ::Selection* view_selection_;

  public: TextSelection(TextWindow* text_window, Range* range);
  public: virtual ~TextSelection();

  public: Posn active() const;
  public: Range* range() const { return range_.get(); }
  public: bool start_is_active() const;
  public: void set_start_is_active(bool start_is_active);
  public: ::Selection* view_selection() const { return view_selection_; }

  DISALLOW_COPY_AND_ASSIGN(TextSelection);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_text_selection_h)
