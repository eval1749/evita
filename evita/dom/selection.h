// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_selection_h)
#define INCLUDE_evita_dom_selection_h

#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

class Selection;

namespace dom {

class Document;
class Range;
class TextWindow;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection : public v8_glue::Scriptable<Selection> {
  private: gc::Member<Document> document_;
  private: gc::Member<TextWindow> text_window_;
  // TODO(yosi): We should remove ::Selection.
  private: ::Selection* view_selection_;
  private: gc::Member<Range> range_;

  public: Selection(TextWindow* text_window, Range* range);
  public: virtual ~Selection();

  public: Document* document() const { return document_.get(); }
  public: Range* range() const { return range_.get(); }
  public: bool start_is_active() const;
  public: void set_start_is_active(bool start_is_active);
  public: static v8_glue::WrapperInfo* static_wrapper_info();
  public: ::Selection* view_selection() const { return view_selection_; }
  public: TextWindow* window() const { return text_window_; }

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_selection_h)
