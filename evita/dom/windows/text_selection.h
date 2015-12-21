// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_TEXT_SELECTION_H_
#define EVITA_DOM_WINDOWS_TEXT_SELECTION_H_

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

 public:
  TextSelection(TextWindow* text_window, Range* range);
  ~TextSelection() final;

  text::Offset anchor_offset() const;
  text::Offset focus_offset() const;
  Range* range() const { return range_.get(); }
  bool start_is_active() const;
  void set_start_is_active(bool start_is_active);
  text::Selection* text_selection() const { return text_selection_; }

 private:
  friend class bindings::TextSelectionClass;

  // For IDL
  int anchor_offset_value() const;
  int focus_offset_value() const;

  // text::SelectionChangeObserver
  void DidChangeSelection() final;

  text::Selection* const text_selection_;
  gc::Member<Range> range_;

  DISALLOW_COPY_AND_ASSIGN(TextSelection);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TEXT_SELECTION_H_
