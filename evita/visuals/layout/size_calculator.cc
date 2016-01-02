// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/layout/size_calculator.h"

#include "evita/visuals/fonts/text_format.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ExtrinsicSizeVisitor
//
class ExtrinsicSizeVisitor final : public BoxVisitor {
 public:
  ExtrinsicSizeVisitor() = default;
  ~ExtrinsicSizeVisitor() final = default;

  FloatSize ComputeExtrinsicSize(const Box& box);

 private:
  void ComputeWithSimpleMethod(const Box& box);
  void ReturnSize(float width, float height);
  void ReturnSize(const FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(ExtrinsicSizeVisitor);
};

FloatSize ExtrinsicSizeVisitor::ComputeExtrinsicSize(const Box& box) {
  if (box.is_display_none())
    return FloatSize();
  Visit(const_cast<Box*>(&box));
  return result_;
}

void ExtrinsicSizeVisitor::ComputeWithSimpleMethod(const Box& box) {
  if (box.height().is_length() && box.width().is_length()) {
    ReturnSize(box.width().length().value(), box.height().length().value());
    return;
  }
  const auto& intrinsic_size = SizeCalculator().ComputeIntrinsicSize(box);
  if (box.height().is_length())
    return ReturnSize(intrinsic_size.width(), box.height().length().value());
  if (box.width().is_length())
    return ReturnSize(box.width().length().value(), intrinsic_size.height());
  ReturnSize(intrinsic_size);
}

void ExtrinsicSizeVisitor::ReturnSize(float width, float height) {
  ReturnSize(FloatSize(width, height));
}

void ExtrinsicSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void ExtrinsicSizeVisitor::VisitBlockBox(BlockBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitLineBox(LineBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitRootBox(RootBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitTextBox(TextBox* box) {
  ComputeWithSimpleMethod(*box);
}

//////////////////////////////////////////////////////////////////////
//
// IntrinsicSizeVisitor
//
class IntrinsicSizeVisitor final : public BoxVisitor {
 public:
  IntrinsicSizeVisitor() = default;
  ~IntrinsicSizeVisitor() final = default;

  FloatSize ComputeIntrinsicSize(const Box& box);

  // Shortcut
  FloatSize ComputePreferredSize(const Box& box);

 private:
  void ReturnSize(const FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(IntrinsicSizeVisitor);
};

FloatSize IntrinsicSizeVisitor::ComputeIntrinsicSize(const Box& box) {
  if (box.is_display_none())
    return FloatSize();
  Visit(const_cast<Box*>(&box));
  return result_;
}

FloatSize IntrinsicSizeVisitor::ComputePreferredSize(const Box& box) {
  return SizeCalculator().ComputePreferredSize(box);
}

void IntrinsicSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void IntrinsicSizeVisitor::VisitBlockBox(BlockBox* box) {
  auto size = FloatSize();
  for (const auto& child : box->child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child) +
                             child->border().size() + child->padding().size();
    size = FloatSize(std::max(size.width(), child_size.width()),
                     size.height() + child_size.height());
  }
  ReturnSize(size);
}

void IntrinsicSizeVisitor::VisitLineBox(LineBox* box) {
  auto size = FloatSize();
  for (const auto& child : box->child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child) +
                             child->border().size() + child->padding().size();
    size = FloatSize(size.width() + child_size.width(),
                     std::max(size.height(), child_size.height()));
  }
  ReturnSize(size);
}

void IntrinsicSizeVisitor::VisitRootBox(RootBox* box) {
  NOTREACHED() << "NYI IntrinsicSizeVisitor for RootBox";
}

void IntrinsicSizeVisitor::VisitTextBox(TextBox* box) {
  if (box->text().empty())
    return ReturnSize(FloatSize());
  // TODO(eval1749): Implement computing preferred size for |TextBox|.
  const auto font_size = 16.0f;  // 12pt * 96dpi / 72
  const auto& text_format =
      std::make_unique<TextFormat>(L"MS Shell Dlg 2", font_size);
  ReturnSize(text_format->ComputeMetrics(box->text()));
}

//////////////////////////////////////////////////////////////////////
//
// PreferredSizeVisitor
//
class PreferredSizeVisitor final : public BoxVisitor {
 public:
  PreferredSizeVisitor() = default;
  ~PreferredSizeVisitor() final = default;

  FloatSize ComputePreferredSize(const Box& box);

 private:
  void ReturnSize(const FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(PreferredSizeVisitor);
};

FloatSize PreferredSizeVisitor::ComputePreferredSize(const Box& box) {
  if (box.is_display_none())
    return FloatSize();
  Visit(const_cast<Box*>(&box));
  return result_;
}

void PreferredSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void PreferredSizeVisitor::VisitBlockBox(BlockBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

void PreferredSizeVisitor::VisitLineBox(LineBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

void PreferredSizeVisitor::VisitRootBox(RootBox* box) {
  ReturnSize(box->viewport_size());
}

void PreferredSizeVisitor::VisitTextBox(TextBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// SizeCalculator
//
SizeCalculator::SizeCalculator() {}
SizeCalculator::~SizeCalculator() {}

FloatSize SizeCalculator::ComputeExtrinsicSize(const Box& box) const {
  return ExtrinsicSizeVisitor().ComputeExtrinsicSize(box);
}

FloatSize SizeCalculator::ComputeIntrinsicSize(const Box& box) const {
  return IntrinsicSizeVisitor().ComputeIntrinsicSize(box);
}

FloatSize SizeCalculator::ComputePreferredSize(const Box& box) const {
  return PreferredSizeVisitor().ComputePreferredSize(box);
}

}  // namespace visuals
