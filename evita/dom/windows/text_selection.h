// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_windows_text_selection_h)
#define INCLUDE_evita_dom_windows_text_selection_h

#include "evita/dom/windows/selection.h"
#include "evita/gc/member.h"
#include "evita/text/selection_change_observer.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Selection;
}

namespace dom {

class Document;
class Range;
class TextWindow;

namespace bindings {
class TextSelectionClass;
}

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
class TextSelection final
    : public v8_glue::Scriptable<TextSelection, Selection>,
      private text::SelectionChangeObserver {
  DECLARE_SCRIPTABLE_OBJECT(TextSelection);
  friend class bindings::TextSelectionClass;

  private: text::Selection* const text_selection_;
  private: gc::Member<Range> range_;

  public: TextSelection(TextWindow* text_window, Range* range);
  public: ~TextSelection() final;

  public: Posn anchor_offset() const;
  public: Posn focus_offset() const;
  public: Range* range() const { return range_.get(); }
  public: bool start_is_active() const;
  public: void set_start_is_active(bool start_is_active);
  public: text::Selection* text_selection() const { return text_selection_; }

  // text::SelectionChangeObserver
  private: void DidChangeSelection() final;

  DISALLOW_COPY_AND_ASSIGN(TextSelection);
};

} // namespace dom

#endif // !defined(INCLUDE_evita_dom_windows_text_selection_h)
