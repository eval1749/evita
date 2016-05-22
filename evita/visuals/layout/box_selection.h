// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_SELECTION_H_
#define EVITA_VISUALS_LAYOUT_BOX_SELECTION_H_

#include "evita/css/values.h"
#include "evita/gfx/base/colors/float_color.h"

namespace visuals {

class Box;
class SelectionEdtior;

//////////////////////////////////////////////////////////////////////
//
// BoxSelection
//
class BoxSelection final {
 public:
  BoxSelection(const BoxSelection& other);
  BoxSelection();
  ~BoxSelection();

  bool operator==(const BoxSelection& other) const;
  bool operator!=(const BoxSelection& other) const;

  Box* anchor_box() const { return anchor_box_; }
  int anchor_offset() const { return anchor_offset_; }
  const gfx::FloatColor& caret_color() const { return caret_color_; }
  const css::CaretShape& caret_shape() const { return caret_shape_; }
  Box* focus_box() const { return focus_box_; }
  int focus_offset() const { return focus_offset_; }
  const gfx::FloatColor& selection_color() const { return selection_color_; }

  bool is_caret() const;
  bool is_none() const;
  bool is_range() const;

 private:
  friend class BoxSelectionEditor;

  Box* anchor_box_ = nullptr;
  int anchor_offset_ = 0;
  gfx::FloatColor caret_color_;
  css::CaretShape caret_shape_;
  Box* focus_box_ = nullptr;
  int focus_offset_ = 0;
  gfx::FloatColor selection_color_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_SELECTION_H_
