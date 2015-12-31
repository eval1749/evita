// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_H_
#define EVITA_VISUALS_MODEL_BOX_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "common/castable.h"
#include "evita/visuals/css/background.h"
#include "evita/visuals/css/border.h"
#include "evita/visuals/css/float_color.h"
#include "evita/visuals/css/margin.h"
#include "evita/visuals/css/padding.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/box_forward.h"

namespace visuals {

class BoxEditor;

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
class Box : public common::Castable {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(Box, Castable);

 public:
  class Ancestors;
  class AncestorsOrSelf;

  virtual ~Box();

  // Box identification number
  int id() const { return id_; }

  // Box tree related values
  ContainerBox* parent() const { return parent_; }

  // Layout related values
  const FloatRect& bounds() const { return bounds_; }
  FloatRect content_bounds() const;

  // CSS background and background
  const css::Background& background() const { return background_; }
  const css::Border& border() const { return border_; }

  // CSS Box model related values
  const css::Margin& margin() const { return margin_; }
  const css::Padding& padding() const { return padding_; }

  // CSS Formatting model
  bool is_display_none() const { return is_display_none_; }

  // CSS Position
  const css::Bottom& bottom() const { return bottom_; }
  const css::Left& left() const { return left_; }
  const css::Position& position() const { return position_; }
  const css::Right& right() const { return right_; }
  const css::Top& top() const { return top_; }

  virtual void Accept(BoxVisitor* visitor) = 0;
  virtual FloatSize ComputePreferredSize() const = 0;
  std::unique_ptr<css::Style> ComputeActualStyle() const;

  bool IsDescendantOf(const Box& other) const;

  // Paint
  bool IsContentClean() const { return !is_content_dirty_; }
  bool IsContentDirty() const { return is_content_dirty_; }

  // Layout
  bool IsLayoutClean() const { return !is_layout_dirty_; }
  bool IsLayoutDirty() const { return is_layout_dirty_; }

 protected:
  Box();

 private:
  css::Background background_;
  css::Border border_;
  // TODO(eval1749): We should incorporate |bottom_| to layout.
  css::Bottom bottom_;
  FloatRect bounds_;
  bool is_display_none_ = false;
  const int id_;
  // When |is_content_dirty_| is true, we send bounds of this box to
  // compositor to display on screen.
  bool is_content_dirty_ = true;
  bool is_layout_dirty_ = true;
  css::Left left_;
  css::Margin margin_;
  css::Padding padding_;
  css::Position position_;
  ContainerBox* parent_ = nullptr;
  // TODO(eval1749): We should incorporate |right_| to layout.
  css::Right right_;
  css::Top top_;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

std::ostream& operator<<(std::ostream& ostream, const Box& box);
std::ostream& operator<<(std::ostream& ostream, const Box* box);

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_H_
