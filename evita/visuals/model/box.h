// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_H_
#define EVITA_VISUALS_MODEL_BOX_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "common/castable.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/box_forward.h"
#include "evita/visuals/style/background.h"
#include "evita/visuals/style/border.h"
#include "evita/visuals/style/float_color.h"
#include "evita/visuals/style/margin.h"
#include "evita/visuals/style/padding.h"

namespace visuals {

class BoxEditor;
enum class Display;
class Style;

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
  const Background& background() const { return background_; }
  const Border& border() const { return border_; }

  // CSS Box model related values
  const Margin& margin() const { return margin_; }
  const Padding& padding() const { return padding_; }

  virtual void Accept(BoxVisitor* visitor) = 0;
  virtual FloatSize ComputePreferredSize() const = 0;
  std::unique_ptr<Style> ComputeActualStyle() const;

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
  Background background_;
  Border border_;
  FloatRect bounds_;
  const int id_;
  // When |is_content_dirty_| is true, we send bounds of this box to
  // compositor to display on screen.
  bool is_content_dirty_ = true;
  bool is_layout_dirty_ = true;
  Margin margin_;
  Padding padding_;
  ContainerBox* parent_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

std::ostream& operator<<(std::ostream& ostream, const Box& box);
std::ostream& operator<<(std::ostream& ostream, const Box* box);

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_H_
