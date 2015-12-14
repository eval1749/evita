// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_RENDER_SELECTION_H_
#define EVITA_LAYOUT_RENDER_SELECTION_H_

#include "evita/css/style.h"
#include "evita/gfx/color_f.h"
#include "evita/text/offset.h"
#include "evita/ui/base/selection_state.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// TextSelectionModel
//
class TextSelectionModel final {
 public:
  typedef ui::SelectionState State;

  TextSelectionModel(State state, text::Offset anchor, text::Offset focus);
  TextSelectionModel(const TextSelectionModel& model);
  TextSelectionModel();
  ~TextSelectionModel();

  TextSelectionModel& operator=(const TextSelectionModel& other);

  bool operator==(const TextSelectionModel& other) const;
  bool operator!=(const TextSelectionModel& other) const;

  text::Offset end() const { return end_; }
  text::Offset focus_offset() const { return focus_offset_; }
  text::Offset start() const { return start_; }

  bool disabled() const { return state_ == State::Disabled; }
  bool has_focus() const { return state_ == State::HasFocus; }
  bool is_caret() const { return start_ == end_; }
  bool is_range() const { return start_ != end_; }

 private:
  text::Offset anchor_offset() const;

  // For fast access to start and end positions of selection,
  // |TextSelectionModel| has |start_| and |end_| instead of anchor offset.
  text::Offset end_;
  text::Offset focus_offset_;
  State state_;
  text::Offset start_;
};

class TextSelection final {
 public:
  TextSelection(const TextSelectionModel& model, const gfx::ColorF& color);
  TextSelection(const TextSelection& other);
  TextSelection();
  ~TextSelection();

  TextSelection& operator=(const TextSelection& other);

  bool operator==(const TextSelection& other) const;
  bool operator!=(const TextSelection& other) const;

  gfx::ColorF color() const { return color_; }
  text::Offset end() const { return model_.end(); }
  text::Offset focus_offset() const { return model_.focus_offset(); }
  text::Offset start() const { return model_.start(); }

  bool disabled() const { return model_.disabled(); }
  bool has_focus() const { return model_.has_focus(); }
  bool is_caret() const { return model_.is_caret(); }
  bool is_range() const { return model_.is_range(); }

 private:
  gfx::ColorF color_;
  TextSelectionModel model_;
};

}  // namespace layout

#endif  // EVITA_LAYOUT_RENDER_SELECTION_H_
