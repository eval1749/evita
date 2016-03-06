// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/box.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/image_box.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"
#include "evita/visuals/layout/shape_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ActualStyleVisitor
//
class ActualStyleVisitor final : public BoxVisitor {
 public:
  ActualStyleVisitor() = default;
  ~ActualStyleVisitor() final = default;

  std::unique_ptr<css::Style> Compute(const Box& box);

 private:
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  css::StyleBuilder builder_;

  DISALLOW_COPY_AND_ASSIGN(ActualStyleVisitor);
};

#define FOR_EACH_PROPERTY_INITIAL_IS_ZERO(V) \
  V(Border, border)                          \
  V(Margin, margin)                          \
  V(Padding, padding)

#define BorderBottom BorderBottomWidth
#define BorderLeft BorderLeftWidth
#define BorderRight BorderRightWidth
#define BorderTop BorderTopWidth

#define FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V) \
  V(Bottom, bottom)                          \
  V(Height, height)                          \
  V(Left, left)                              \
  V(Right, right)                            \
  V(Top, top)                                \
  V(Width, width)

std::unique_ptr<css::Style> ActualStyleVisitor::Compute(const Box& box) {
  builder_.SetDisplay(box.display());
  if (box.background_color() != FloatColor())
    builder_.Set(css::PropertyId::BackgroundColor,
                 css::ColorValue(box.background_color()));

  const auto& border = box.border();
  const auto& margin = box.margin();
  const auto& padding = box.padding();

// TODO(eval1749): Export border colors
#define V(Property, property)                       \
  if (property.bottom() != 0.0f)                    \
    builder_.Set(css::PropertyId::Property##Bottom, \
                 css::Length(property.bottom()));   \
  if (property.left() != 0.0f)                      \
    builder_.Set(css::PropertyId::Property##Left,   \
                 css::Length(property.left()));     \
  if (property.right() != 0.0f)                     \
    builder_.Set(css::PropertyId::Property##Right,  \
                 css::Length(property.right()));    \
  if (property.top() != 0.0f)                       \
    builder_.Set(css::PropertyId::Property##Top, css::Length(property.top()));
  FOR_EACH_PROPERTY_INITIAL_IS_ZERO(V)
#undef V

  // CSS Position
  if (!box.position().is_static())
    builder_.SetPosition(box.position());

#define V(Property, property)    \
  if (!box.property().is_auto()) \
    builder_.Set##Property(box.property());
  FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V)
#undef V

  Visit(box);
  return std::move(builder_.Build());
}

void ActualStyleVisitor::VisitFlowBox(FlowBox* flow_box) {}

void ActualStyleVisitor::VisitImageBox(ImageBox* image) {}

void ActualStyleVisitor::VisitRootBox(RootBox* root) {}

void ActualStyleVisitor::VisitShapeBox(ShapeBox* shape) {
  builder_.Set(css::PropertyId::Color, css::ColorValue(shape->color()));
}

void ActualStyleVisitor::VisitTextBox(TextBox* text) {
  builder_.Set(css::PropertyId::Color, css::ColorValue(text->color()));
  // TODO(eval1749): We should add font properties
}

}  // namespace

std::unique_ptr<css::Style> Box::ComputeActualStyle() const {
  ActualStyleVisitor builder;
  return builder.Compute(*this);
}

}  // namespace visuals
