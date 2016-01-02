// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/node.h"

#include "evita/visuals/dom/node_visitor.h"
#include "evita/visuals/dom/text_node.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ActualStyleVisitor
//
class ActualStyleVisitor final : public NodeVisitor {
 public:
  ActualStyleVisitor() = default;
  ~ActualStyleVisitor() final = default;

  std::unique_ptr<css::Style> Compute(const Node& node);

 private:
#define V(name) void Visit##name(name* node) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  css::StyleBuilder builder_;

  DISALLOW_COPY_AND_ASSIGN(ActualStyleVisitor);
};

#define FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V) \
  V(Bottom, bottom)                          \
  V(Height, height)                          \
  V(Left, left)                              \
  V(Right, right)                            \
  V(Top, top)                                \
  V(Width, width)

std::unique_ptr<css::Style> ActualStyleVisitor::Compute(const Node& node) {
  if (node.background().HasValue())
    builder_.SetBackground(node.background());
  if (node.border().HasValue())
    builder_.SetBorder(node.border());
  if (node.is_display_none())
    builder_.SetDisplay(css::Display::None());
  if (node.margin().HasValue())
    builder_.SetMargin(node.margin());
  if (node.padding().HasValue())
    builder_.SetPadding(node.padding());

  // CSS Position
  if (!node.position().is_static())
    builder_.SetPosition(node.position());

#define V(Property, property)     \
  if (!node.property().is_auto()) \
    builder_.Set##Property(node.property());
  FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V)
#undef V

  Visit(node);
  return std::move(builder_.Build());
}

void ActualStyleVisitor::VisitDocument(Document* root) {}

void ActualStyleVisitor::VisitElement(Element* line) {}

void ActualStyleVisitor::VisitTextNode(TextNode* text) {
  builder_.SetColor(css::Color(text->color()));
}

}  // namespace

std::unique_ptr<css::Style> Node::ComputeActualStyle() const {
  ActualStyleVisitor builder;
  return builder.Compute(*this);
}

}  // namespace visuals
