// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_selection_h)
#define INCLUDE_evita_views_text_render_selection_h

#include "evita/css/style.h"
#include "evita/gfx/color_f.h"
#include "evita/ui/base/selection_state.h"

namespace views {
namespace rendering {

class TextSelectionModel final {
  public: typedef ui::SelectionState State;

  // For fast access to start and end positions of selection,
  // |TextSelectionModel| has |start_| and |end_| instead of anchor offset.
  private: text::Posn end_;
  private: text::Posn focus_offset_;
  private: State state_;
  private: text::Posn start_;

  public: TextSelectionModel(State state, text::Posn anchor, text::Posn focus);
  public: TextSelectionModel(const TextSelectionModel& model);
  public: TextSelectionModel();
  public: ~TextSelectionModel();

  public: bool operator==(const TextSelectionModel& other) const;
  public: bool operator!=(const TextSelectionModel& other) const;

  private: text::Posn anchor_offset() const;
  public: text::Posn end() const { return end_; }
  public: text::Posn focus_offset() const { return focus_offset_; }
  public: text::Posn start() const { return start_; }

  public: bool disabled() const { return state_ == State::Disabled; }
  public: bool has_focus() const { return state_ == State::HasFocus; }
  public: bool is_caret() const { return start_ == end_; }
  public: bool is_range() const { return start_ != end_; }
};

class TextSelection final {
  private: gfx::ColorF color_;
  private: TextSelectionModel model_;

  public: TextSelection(const TextSelectionModel& model,
                        const gfx::ColorF& color);
  public: TextSelection(const TextSelection& other);
  public: TextSelection();
  public: ~TextSelection();

  bool operator==(const TextSelection& other) const;
  bool operator!=(const TextSelection& other) const;

  public: gfx::ColorF color() const { return color_; }
  public: text::Posn end() const { return model_.end(); }
  public: text::Posn focus_offset() const { return model_.focus_offset(); }
  public: text::Posn start() const { return model_.start(); }

  public: bool disabled() const { return model_.disabled(); }
  public: bool has_focus() const { return model_.has_focus(); }
  public: bool is_caret() const { return model_.is_caret(); }
  public: bool is_range() const { return model_.is_range(); }
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_selection_h)
