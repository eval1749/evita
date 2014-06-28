// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_selection_h)
#define INCLUDE_evita_views_text_render_selection_h

#include "evita/css/style.h"
#include "evita/gfx/color_f.h"

namespace views {
namespace rendering {

class TextSelectionModel final {
  public: enum class Active {
    NotActive,
    EndIsActive,
    RangeIsActive,
    StartIsActive,
  };
  private: Active active_;
  private: text::Posn end_;
  private: text::Posn start_;

  public: TextSelectionModel(text::Posn start, text::Posn end,
                             Active active);
  public: TextSelectionModel(const TextSelectionModel& model);
  public: TextSelectionModel();
  public: ~TextSelectionModel();

  public: bool operator==(const TextSelectionModel& other) const;
  public: bool operator!=(const TextSelectionModel& other) const;

  public: text::Posn end() const { return end_; }
  public: text::Posn start() const { return start_; }

  public: text::Posn active_offset() const;
  public: bool has_caret() const;
  public: bool is_active() const { return active_ != Active::NotActive; }
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

  public: gfx::ColorF color() const { return color_; }
  public: text::Posn end() const { return model_.end(); }
  public: text::Posn start() const { return model_.start(); }

  public: text::Posn active_offset() const { return model_.active_offset(); }
  public: bool has_caret() const { return model_.has_caret(); }
  public: bool is_active() const { return model_.is_active(); }
  public: bool is_caret() const { return model_.is_caret(); }
  public: bool is_range() const { return model_.is_range(); }

  bool operator==(const TextSelection& other) const;
  bool operator!=(const TextSelection& other) const;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_selection_h)
