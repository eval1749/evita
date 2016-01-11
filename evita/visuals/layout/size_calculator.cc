// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/layout/size_calculator.h"

#include "base/logging.h"
#include "evita/visuals/fonts/text_format.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

namespace {

// TODO(eval1749) We should move |IsDisplayOutsideInline()| to |css::Display|.
bool IsDisplayOutsideInline(const css::Display& display) {
  return display.is_inline() || display.is_inline_block();
}

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
void ExtrinsicSizeVisitor::VisitFlowBox(FlowBox* box) {
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
  FloatSize SizeOfHorizontalFlowBox(const FlowBox& flow_box);
  void ReturnSize(const FloatSize& size);
  FloatSize SizeOfVerticalFlowBox(const FlowBox& flow_box);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(IntrinsicSizeVisitor);
};

FloatSize IntrinsicSizeVisitor::ComputeIntrinsicSize(const Box& box) {
  Visit(const_cast<Box*>(&box));
  return result_;
}

FloatSize IntrinsicSizeVisitor::ComputePreferredSize(const Box& box) {
  return SizeCalculator().ComputePreferredSize(box);
}

void IntrinsicSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

FloatSize IntrinsicSizeVisitor::SizeOfHorizontalFlowBox(
    const FlowBox& flow_box) {
  DCHECK(flow_box.first_child()) << flow_box;
  auto size = FloatSize();
  for (const auto& child : flow_box.child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child) +
                             child->border().size() + child->padding().size();
    size = FloatSize(size.width() + child_size.width(),
                     std::max(size.height(), child_size.height()));
  }
  return size;
}

FloatSize IntrinsicSizeVisitor::SizeOfVerticalFlowBox(const FlowBox& flow_box) {
  DCHECK(flow_box.first_child()) << flow_box;
  auto size = FloatSize();
  for (const auto& child : flow_box.child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child) +
                             child->border().size() + child->padding().size();
    size = FloatSize(std::max(size.width(), child_size.width()),
                     size.height() + child_size.height());
  }
  return size;
}

// BoxVisitor
void IntrinsicSizeVisitor::VisitFlowBox(FlowBox* flow_box) {
  const auto first_child = flow_box->first_child();
  if (!first_child)
    return ReturnSize(FloatSize());
  if (IsDisplayOutsideInline(first_child->display()))
    return ReturnSize(SizeOfHorizontalFlowBox(*flow_box));
  return ReturnSize(SizeOfVerticalFlowBox(*flow_box));
}

void IntrinsicSizeVisitor::VisitRootBox(RootBox* box) {
  NOTREACHED() << "NYI IntrinsicSizeVisitor for RootBox";
}

void IntrinsicSizeVisitor::VisitTextBox(TextBox* box) {
  if (box->text().empty())
    return ReturnSize(FloatSize());

  const auto& text_format = BoxEditor().EnsureTextFormat(box);
  ReturnSize(text_format.ComputeMetrics(box->text()));
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
  Visit(const_cast<Box*>(&box));
  return result_;
}

void PreferredSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void PreferredSizeVisitor::VisitFlowBox(FlowBox* box) {
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
