// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_selection_h)
#define INCLUDE_evita_views_text_render_selection_h

#include "evita/css/style.h"
#include "evita/gfx/color_f.h"

namespace views {
namespace rendering {

struct TextSelectionModel {
  bool active;
  text::Posn start;
  text::Posn end;

  TextSelectionModel(text::Posn start, text::Posn end, bool active);
  TextSelectionModel(const TextSelectionModel& model);
  TextSelectionModel();
  ~TextSelectionModel();

  bool operator==(const TextSelectionModel& other) const;
  bool operator!=(const TextSelectionModel& other) const;

  bool is_caret() const { return start == end; }
  bool is_range() const { return start != end; }
};

struct TextSelection : TextSelectionModel {
  gfx::ColorF color;

  TextSelection(const TextSelectionModel& model, const gfx::ColorF& color);
  TextSelection();
  ~TextSelection();

  bool operator==(const TextSelection& other) const;
  bool operator!=(const TextSelection& other) const;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_selection_h)
