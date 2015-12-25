// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_H_
#define EVITA_VISUALS_MODEL_BOX_H_

#include <iosfwd>

#include "base/macros.h"
#include "common/castable.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/box_forward.h"

namespace visuals {

enum class Display;

#define DECLARE_VISUAL_BOX_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)

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
  virtual ~Box();

  const FloatRect& bounds() const { return bounds_; }
  ContainerBox* parent() const { return parent_; }

  virtual void Accept(BoxVisitor* visitor) = 0;
  bool IsBlock() const;
  bool IsDescendantOf(const Box& other) const;
  bool IsInline() const;

 protected:
  Box(Display display, const FloatRect& bounds);

 private:
  class Editor;

  FloatRect bounds_;
  bool dirty_ = true;
  const Display display_;
  ContainerBox* parent_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

//////////////////////////////////////////////////////////////////////
//
// Box::Editor
//
class Box::Editor final {
 public:
  explicit Editor(Box* box);
  ~Editor();

  void SetParent(ContainerBox* parentBox);

 private:
  Box* const box_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

std::ostream& operator<<(std::ostream& ostream, const Box& box);
std::ostream& operator<<(std::ostream& ostream, const Box* box);

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_H_
