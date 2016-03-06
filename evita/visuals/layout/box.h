// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_H_
#define EVITA_VISUALS_LAYOUT_BOX_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "common/castable.h"
#include "evita/visuals/css/float_color.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/box_forward.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"

namespace visuals {

class BoxEditor;
class Node;
class RootBox;

namespace css {
class Style;
}

#define DECLARE_VISUAL_BOX_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)         \
  friend class BoxEditor;

#define DECLARE_VISUAL_BOX_ABSTRACT_CLASS(self, super) \
  DECLARE_VISUAL_BOX_CLASS(self, super)

#define DECLARE_VISUAL_BOX_FINAL_CLASS(self, super) \
  DECLARE_VISUAL_BOX_CLASS(self, super)             \
 private:                                           \
  void Accept(BoxVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Box
//
class Box : public common::Castable<Box> {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(Box, Castable);

 public:
  class Ancestors;
  class AncestorsOrSelf;
  class Children;
  class Descendants;
  class DescendantsOrSelf;

  virtual ~Box();

  // Tree structure
  Box* next_sibling() const { return next_sibling_; }
  ContainerBox* parent() const { return parent_; }
  Box* previous_sibling() const { return previous_sibling_; }
  RootBox* root_box() const { return root_box_; }
  int version() const { return version_; }

  // Box identifiers
  const Node* node() const { return node_; }
  // |sequence_id()| is used for debugging display tree generation and display.
  int sequence_id() const { return sequence_id_; }

  // Layout related values
  const FloatRect& bounds() const { return bounds_; }
  FloatRect content_bounds() const;
  const css::Display& display() const { return display_; }

  bool is_changed() const { return is_changed_; }

  // CSS background and background
  const FloatColor& background_color() const { return background_color_; }
  const Border border() const { return border_; }

  // CSS Box model related values
  const Margin& margin() const { return margin_; }
  const Padding& padding() const { return padding_; }

  // CSS Position
  const css::Bottom& bottom() const { return bottom_; }
  const css::Left& left() const { return left_; }
  const css::Position& position() const { return position_; }
  const css::Right& right() const { return right_; }
  const css::Top& top() const { return top_; }

  // CSS Sizing
  const css::Height height() const { return height_; }
  const css::Width width() const { return width_; }

  virtual void Accept(BoxVisitor* visitor) = 0;
  std::unique_ptr<css::Style> ComputeActualStyle() const;

  bool InDocument() const;
  bool IsDescendantOf(const Box& other) const;

  // Change tracking
  bool IsBackgroundChanged() const { return is_background_changed_; }
  bool IsBorderChanged() const { return is_border_changed_; }
  bool IsContentChanged() const { return is_content_changed_; }
  bool IsOriginChanged() const { return is_origin_changed_; }
  bool IsPaddingChanged() const { return is_padding_changed_; }
  bool IsSizeChanged() const { return is_size_changed_; }

  // Paint
  bool ShouldPaint() const { return should_paint_; }

 protected:
  Box(RootBox* root_box, const Node* node);
  explicit Box(RootBox* root_box);

 private:
  virtual void DidChangeBounds(const FloatRect& old_bounds);

  FloatRect bounds_;
  const Node* const node_;
  const int sequence_id_;

  bool is_background_changed_ = true;
  bool is_border_changed_ = true;
  bool is_changed_ = true;
  // When |is_content_changed_| is true, we send bounds of this box to
  // compositor to display on screen. Note: On |ContainerBox|,
  // |is_content_dirty| is always false.
  bool is_content_changed_ = false;
  bool is_origin_changed_ = true;
  bool is_padding_changed_ = true;
  bool is_size_changed_ = true;
  bool should_paint_ = true;

  FloatColor background_color_;

  Border border_;
  FloatColor border_bottom_color_;
  FloatColor border_left_color_;
  FloatColor border_right_color_;
  FloatColor border_top_color_;

  css::Display display_;
  Margin margin_;
  Padding padding_;

  // CSS Position
  css::Position position_;
  // TODO(eval1749): We should have |bool| for having bottom/left/right/top and
  // |float| instead of CSS values.
  css::Bottom bottom_;
  css::Left left_;
  css::Right right_;
  css::Top top_;

  // CSS Sizing
  // TODO(eval1749): We should have |bool| for having width/height and
  // |float| instead of CSS values.
  css::Width width_;
  css::Height height_;

  // Tree structure
  Box* next_sibling_ = nullptr;
  Box* previous_sibling_ = nullptr;
  ContainerBox* parent_ = nullptr;
  RootBox* const root_box_;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

std::ostream& operator<<(std::ostream& ostream, const Box& box);
std::ostream& operator<<(std::ostream& ostream, const Box* box);

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_H_
