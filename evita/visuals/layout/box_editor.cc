// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <string>

#include "evita/visuals/layout/box_editor.h"

#include "base/logging.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/fonts/font_description_builder.h"
#include "evita/visuals/fonts/text_format_factory.h"
#include "evita/visuals/fonts/text_layout.h"
#include "evita/visuals/imaging/image_bitmap.h"
#include "evita/visuals/layout/ancestors.h"
#include "evita/visuals/layout/ancestors_or_self.h"
#include "evita/visuals/layout/box_selection.h"
#include "evita/visuals/layout/descendants_or_self.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/image_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/shape_box.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

namespace {

gfx::FloatColor ResolveColor(const css::Style& style, const css::Value& value) {
  if (value.is_color())
    return value.as_color_value().value();
  if (value.is_current_color())
    return style.color().as_color_value().value();
  NOTREACHED() << value;
  return gfx::FloatColor();
}

float ResolveLength(const css::Style& style, const css::Value& value) {
  DCHECK_EQ(css::Unit::px, value.as_length().unit());
  return value.as_length().number();
}

float ResolveLineWidth(const css::Style& style, const css::Value& value) {
  if (value.is_length()) {
    DCHECK_EQ(css::Unit::px, value.as_length().unit()) << value;
    return value.as_length().number();
  }
  const auto font_size = style.font_size().as_length().number();
  DCHECK_EQ(css::Unit::px, style.font_size().as_length().unit())
      << style.font_size();
  if (value.is_medium())
    return std::min(std::floor(font_size / 5), 1.0f);
  if (value.is_thick())
    return std::min(std::floor(font_size / 3), 1.0f);
  if (value.is_thin())
    return std::min(std::floor(font_size / 10), 1.0f);
  return 0;
}

Border ComputeBorder(const css::Style& style) {
  gfx::FloatColor bottom_color;
  gfx::FloatColor left_color;
  gfx::FloatColor right_color;
  gfx::FloatColor top_color;
  float bottom_width = 0.0f;
  float left_width = 0.0f;
  float right_width = 0.0f;
  float top_width = 0.0f;

  if (style.has_border_bottom_style() &&
      !style.border_bottom_style().is_none()) {
    bottom_color = ResolveColor(style, style.border_bottom_color().value());
    bottom_width = ResolveLineWidth(style, style.border_bottom_width().value());
  }

  if (style.has_border_left_style() && !style.border_left_style().is_none()) {
    left_color = ResolveColor(style, style.border_left_color().value());
    left_width = ResolveLineWidth(style, style.border_left_width().value());
  }

  if (style.has_border_right_style() && !style.border_right_style().is_none()) {
    right_color = ResolveColor(style, style.border_right_color().value());
    right_width = ResolveLineWidth(style, style.border_right_width().value());
  }

  if (style.has_border_top_style() && !style.border_top_style().is_none()) {
    top_color = ResolveColor(style, style.border_top_color().value());
    top_width = ResolveLineWidth(style, style.border_top_width().value());
  }

  return Border(top_color, top_width, right_color, right_width, bottom_color,
                bottom_width, left_color, left_width);
}

Margin ComputeMargin(const css::Style& style) {
  auto margin_bottom = 0.0f;
  auto margin_left = 0.0f;
  auto margin_right = 0.0f;
  auto margin_top = 0.0f;

  if (style.has_margin_bottom())
    margin_bottom = ResolveLength(style, style.margin_bottom().value());

  if (style.has_margin_left())
    margin_left = ResolveLength(style, style.margin_left().value());

  if (style.has_margin_right())
    margin_right = ResolveLength(style, style.margin_right().value());

  if (style.has_margin_top())
    margin_top = ResolveLength(style, style.margin_top().value());

  return Margin(margin_top, margin_right, margin_bottom, margin_left);
}

Padding ComputePadding(const css::Style& style) {
  auto padding_bottom = 0.0f;
  auto padding_left = 0.0f;
  auto padding_right = 0.0f;
  auto padding_top = 0.0f;

  if (style.has_padding_bottom())
    padding_bottom = ResolveLength(style, style.padding_bottom().value());

  if (style.has_padding_left())
    padding_left = ResolveLength(style, style.padding_left().value());

  if (style.has_padding_right())
    padding_right = ResolveLength(style, style.padding_right().value());

  if (style.has_padding_top())
    padding_top = ResolveLength(style, style.padding_top().value());

  return Padding(padding_top, padding_right, padding_bottom, padding_left);
}

void MustBeInLayout(const Box& box) {
  const auto lifecycle = box.root_box()->lifecycle();
  DCHECK_EQ(ViewLifecycle::State::InLayout, lifecycle->state()) << lifecycle;
}

void MustBeInTreeRebuild(const Box& box) {
  const auto lifecycle = box.root_box()->lifecycle();
  DCHECK_EQ(ViewLifecycle::State::InTreeRebuild, lifecycle->state())
      << lifecycle;
}

void MustBeInPaint(const Box& box) {
  const auto lifecycle = box.root_box()->lifecycle();
  DCHECK_EQ(ViewLifecycle::State::InPaint, lifecycle->state()) << lifecycle;
}

FontStretch ConvertFontStretch(const css::FontStretch stretch) {
  if (stretch.is_condensed())
    return FontStretch::Condensed;
  if (stretch.is_expanded())
    return FontStretch::Condensed;
  if (stretch.is_extra_condensed())
    return FontStretch::ExtraCondensed;
  if (stretch.is_extra_expanded())
    return FontStretch::ExtraExpanded;
  if (stretch.is_normal())
    return FontStretch::Normal;
  if (stretch.is_semi_condensed())
    return FontStretch::Condensed;
  if (stretch.is_semi_expanded())
    return FontStretch::SemiExpanded;
  if (stretch.is_ultra_condensed())
    return FontStretch::UltraCondensed;
  if (stretch.is_ultra_expanded())
    return FontStretch::UltraExpanded;
  NOTREACHED() << "Unsupported font-stretch: " << stretch;
  return FontStretch::Normal;
}

FontStyle ConvertFontStyle(const css::FontStyle& style) {
  if (style.is_italic())
    return FontStyle::Italic;
  if (style.is_normal())
    return FontStyle::Normal;
  if (style.is_oblique())
    return FontStyle::Oblique;
  NOTREACHED() << "Unsupported font-style: " << style;
  return FontStyle::Normal;
}

FontWeight ConvertFontWeight(const css::FontWeight& weight) {
  if (weight.is_bold())
    return FontWeight::Bold;
  if (weight.is_normal())
    return FontWeight::Normal;
  if (weight.is_100())
    return FontWeight::k100;
  if (weight.is_200())
    return FontWeight::k200;
  if (weight.is_300())
    return FontWeight::k300;
  if (weight.is_400())
    return FontWeight::k400;
  if (weight.is_500())
    return FontWeight::k500;
  if (weight.is_600())
    return FontWeight::k600;
  if (weight.is_700())
    return FontWeight::k700;
  if (weight.is_800())
    return FontWeight::k800;
  if (weight.is_900())
    return FontWeight::k900;
  NOTREACHED() << "Unsupported font-weight: " << weight;
  return FontWeight::Normal;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BoxEditor
//
BoxEditor::BoxEditor() {}
BoxEditor::~BoxEditor() {}

void BoxEditor::AllocateTextLayout(TextBox* box) {
  MustBeInLayout(*box);
  if (box->text_layout_)
    return;
  const auto& size = box->content_bounds().size();
  if (size.IsEmpty())
    return;
  box->text_layout_.reset(new TextLayout(*box->text_format_, box->data_, size));
}

void BoxEditor::AppendChild(ContainerBox* container, Box* new_child) {
  MustBeInTreeRebuild(*container);
  DCHECK_NE(container, new_child);
  DCHECK(!new_child->IsDescendantOf(*container));
  DCHECK(!container->IsDescendantOf(*new_child));
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->parent_);
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->next_sibling_);
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->previous_sibling_);
  new_child->parent_ = container;
  if (const auto old_last_child = container->last_child_) {
    new_child->previous_sibling_ = old_last_child;
    old_last_child->next_sibling_ = new_child;
  } else {
    container->first_child_ = new_child;
  }
  container->last_child_ = new_child;
  MarkDirty(container);
  new_child->version_ = container->version_;
}

const FontDescription& BoxEditor::ComputeFontDescription(const TextBox& box) {
  MustBeInLayout(box);
  FontDescription::Builder builder;
  builder.SetFamily(box.font_family_.as_string().data());
  builder.SetSize(box.font_size_.as_length().value());
  builder.SetStretch(ConvertFontStretch(box.font_stretch_));
  builder.SetStyle(ConvertFontStyle(box.font_style_));
  builder.SetWeight(ConvertFontWeight(box.font_weight_));
  return builder.Build();
}

void BoxEditor::DidLayout(Box* box) {
  // TODO(eval1749): What should we do here?
}

void BoxEditor::DidMove(Box* box) {
  box->is_origin_changed_ = true;
  SetShouldPaint(box);
}

void BoxEditor::DidPaint(Box* box) {
  MustBeInPaint(*box);
  DCHECK(box->root_box()->InPaint());
  box->is_background_changed_ = false;
  box->is_border_changed_ = false;
  box->is_content_changed_ = false;
  box->is_origin_changed_ = false;
  box->is_padding_changed_ = false;
  box->is_size_changed_ = false;
  box->should_paint_ = false;

  box->is_changed_ = false;
  const auto container = box->as<ContainerBox>();
  if (!container)
    return;
  container->is_child_changed_ = false;
}

const TextFormat& BoxEditor::EnsureTextFormat(TextBox* box) {
  MustBeInLayout(*box);
  const auto& text_format =
      TextFormatFactory::GetInstance()->Get(ComputeFontDescription(*box));
  if (box->text_format_ == &text_format)
    return text_format;
  box->text_format_ = &text_format;
  box->text_layout_.reset();
  box->preferred_size_ = gfx::FloatSize();
  return text_format;
}

void BoxEditor::MarkDirty(Box* box) {
  ++box->root_box_->version_;
  box->version_ = box->root_box_->version_;
  box->is_changed_ = true;
  for (const auto& runner : Box::Ancestors(*box)) {
    if (runner->is_changed_ || runner->is_child_changed_)
      return;
    runner->is_child_changed_ = true;
  }
}

void BoxEditor::RemoveAllChildren(ContainerBox* container) {
  while (const auto child = container->first_child()) {
    RemoveChild(container, child);
    if (!child->node()) {
      // Delete anonymous box.
      delete child;
    }
  }
}

void BoxEditor::RemoveChild(ContainerBox* container, Box* old_child) {
  const auto root_box = container->root_box();
  if (!root_box->lifecycle()->InShutdown())
    MustBeInTreeRebuild(*container);
  DCHECK_EQ(container, old_child->parent_);
  const auto next_sibling = old_child->next_sibling_;
  const auto previous_sibling = old_child->previous_sibling_;

  if (next_sibling)
    next_sibling->previous_sibling_ = old_child->previous_sibling_;
  else
    container->last_child_ = old_child->previous_sibling_;
  if (previous_sibling)
    previous_sibling->next_sibling_ = next_sibling;
  else
    container->first_child_ = next_sibling;

  old_child->next_sibling_ = nullptr;
  old_child->previous_sibling_ = nullptr;
  old_child->parent_ = nullptr;
  if (root_box->lifecycle()->InShutdown())
    return;
  MarkDirty(container);
}

void BoxEditor::RemoveDescendants(ContainerBox* container_box) {
  while (auto const child = container_box->first_child()) {
    if (const auto child_container_box = child->as<ContainerBox>())
      RemoveDescendants(child_container_box);
    RemoveChild(container_box, child);
  }
}

void BoxEditor::ScheduleForcePaint(RootBox* root_box) {
  root_box->lifecycle()->LimitTo(ViewLifecycle::State::LayoutClean);
}

void BoxEditor::SetBaseline(TextBox* box, float new_baseline) {
  MustBeInLayout(*box);
  if (box->baseline_ == new_baseline)
    return;
  box->baseline_ = new_baseline;
  box->is_content_changed_ = true;
  MarkDirty(box);
}

void BoxEditor::SetBounds(Box* box, const gfx::FloatRect& new_bounds) {
  MustBeInLayout(*box);
  if (box->bounds_ == new_bounds)
    return;
  if (box->bounds_.origin() != new_bounds.origin())
    box->is_origin_changed_ = true;
  if (box->bounds_.size() != new_bounds.size()) {
    box->is_background_changed_ = true;
    if (!box->border().IsEmpty())
      box->is_border_changed_ = true;
    box->is_size_changed_ = true;
  }
  const auto old_bounds = box->bounds_;
  box->bounds_ = new_bounds;
  box->DidChangeBounds(old_bounds);
}

void BoxEditor::SetContentChanged(ContentBox* box) {
  box->is_content_changed_ = true;
  MarkDirty(box);
}

void BoxEditor::SetDisplay(Box* box, const css::Display& display) {
  MustBeInTreeRebuild(*box);
  box->display_ = display;
}

#define FOR_EACH_PROPERTY_AFFECTS_ORIGIN(V) \
  V(bottom)                                 \
  V(left)                                   \
  V(position)                               \
  V(right)                                  \
  V(top)

#define FOR_EACH_PROPERTY_AFFECTS_SIZE(V) \
  V(height)                               \
  V(width)

#define UPDATE_COLOR(property, name)                                           \
  const auto& new_##name## =                                                   \
      new_style.has_##name##() ? new_style.##name##().as_color_value().value() \
                               : gfx::FloatColor();                            \
  if (box->##name##_ != new_##name##) {                                        \
    box->##name##_ = new_##name##;                                             \
    box->is_##property##_changed_ = true;                                      \
    is_changed = true;                                                         \
  }

void BoxEditor::SetStyle(Box* box, const css::Style& new_style) {
  MustBeInTreeRebuild(*box);
  if (const auto& image = box->as<ImageBox>())
    return SetImageStyle(image, new_style);

  if (const auto& shape = box->as<ShapeBox>())
    return SetShapeStyle(shape, new_style);

  if (const auto& text = box->as<TextBox>())
    return SetTextStyle(text, new_style);

  auto is_changed = false;
  if (new_style.has_display() && new_style.display() != box->display()) {
    box->display_ = new_style.display();
    is_changed = true;
  }

  UPDATE_COLOR(background, background_color);
  UPDATE_COLOR(border, border_bottom_color);
  UPDATE_COLOR(border, border_left_color);
  UPDATE_COLOR(border, border_right_color);
  UPDATE_COLOR(border, border_top_color);

  const auto& new_border = ComputeBorder(new_style);
  if (box->border_ != new_border) {
    box->border_ = new_border;
    box->is_border_changed_ = true;
    is_changed = true;
  }

  const auto& new_margin = ComputeMargin(new_style);
  if (box->margin_ != new_margin) {
    box->margin_ = new_margin;
    box->is_origin_changed_ = true;
    is_changed = true;
  }

  const auto& new_padding = ComputePadding(new_style);
  if (box->padding_ != new_padding) {
    box->padding_ = new_padding;
    box->is_padding_changed_ = true;
    is_changed = true;
  }

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_origin_changed_ = true;               \
    is_changed = true;                            \
  }
  FOR_EACH_PROPERTY_AFFECTS_ORIGIN(V)
#undef V

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_size_changed_ = true;                 \
    is_changed = true;                            \
  }
  FOR_EACH_PROPERTY_AFFECTS_SIZE(V)
#undef V

  if (!is_changed)
    return;
  MarkDirty(box);
}

void BoxEditor::SetImageData(ImageBox* image_box, const ImageData& data) {
  MustBeInTreeRebuild(*image_box);
  if (image_box->data_ == data)
    return;
  image_box->data_ = data;
  SetContentChanged(image_box);
}

void BoxEditor::SetImageStyle(ImageBox* box, const css::Style& new_style) {
  MustBeInTreeRebuild(*box);
  auto is_changed = false;

  if (!is_changed)
    return;
  box->is_content_changed_ = true;
  MarkDirty(box);
}

void BoxEditor::SetPreferredSize(TextBox* box, const gfx::FloatSize& size) {
  box->preferred_size_ = size;
}

void BoxEditor::SetShapeData(ShapeBox* shape_box, const ShapeData& data) {
  MustBeInTreeRebuild(*shape_box);
  if (shape_box->data_ == data)
    return;
  shape_box->data_ = data;
  SetContentChanged(shape_box);
}

void BoxEditor::SetShapeStyle(ShapeBox* box, const css::Style& new_style) {
  MustBeInTreeRebuild(*box);
  auto is_changed = false;
  // |ShapeBox| uses only color, ant font related CSS properties.
  const auto& new_color = new_style.has_color()
                              ? new_style.color().as_color_value().value()
                              : gfx::FloatColor();
  if (box->color_ != new_color) {
    box->color_ = new_color;
    is_changed = true;
  }

  if (new_style.has_font_size() && new_style.font_size() != box->font_size_) {
    box->font_size_ = new_style.font_size();
    is_changed = true;
  }

  if (!is_changed)
    return;
  box->is_content_changed_ = true;
  MarkDirty(box);
}

void BoxEditor::SetShouldPaint(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelf(*box)) {
    if (runner->should_paint_)
      return;
    runner->should_paint_ = true;
  }
}

void BoxEditor::SetTextData(TextBox* text_box, base::StringPiece16 data) {
  MustBeInTreeRebuild(*text_box);
  if (text_box->data_ == data)
    return;
  text_box->data_ = data.as_string();
  text_box->text_layout_.reset();
  text_box->preferred_size_ = gfx::FloatSize();
  SetContentChanged(text_box);
}

#define FOR_EACH_PROPERTY_AFFECTS_TEXT_FONT(V) \
  V(font_family)                               \
  V(font_size)                                 \
  V(font_stretch)                              \
  V(font_style)                                \
  V(font_weight)

void BoxEditor::SetTextStyle(TextBox* box, const css::Style& new_style) {
  MustBeInTreeRebuild(*box);
  auto is_changed = false;
  // |TextBox| uses only color, ant font related CSS properties.
  const auto& new_color = new_style.has_color()
                              ? new_style.color().as_color_value().value()
                              : gfx::FloatColor();
  if (box->color_ != new_color) {
    box->color_ = new_color;
    is_changed = true;
  }

#define V(name)                                                     \
  if (new_style.has_##name() && new_style.name() != box->name##_) { \
    box->name##_ = new_style.name();                                \
    box->text_format_ = nullptr;                                    \
    is_changed = true;                                              \
  }
  FOR_EACH_PROPERTY_AFFECTS_TEXT_FONT(V)
#undef V

  if (!is_changed)
    return;
  box->is_content_changed_ = true;
  MarkDirty(box);
}

void BoxEditor::SetSelection(RootBox* root_box,
                             const BoxSelection& new_selection) {
  MustBeInTreeRebuild(*root_box);
  const auto& old_selection = root_box->selection();
  if (old_selection == new_selection)
    return;
  if (old_selection.is_caret()) {
    SetContentChanged(old_selection.focus_box()->as<TextBox>());
  }
  if (old_selection.is_range()) {
    // TODO(eval1749): We should call |SetContentChanged()| between anchor and
    // focus.
    SetContentChanged(old_selection.focus_box()->as<TextBox>());
  }
  if (new_selection.is_caret()) {
    SetContentChanged(new_selection.focus_box()->as<TextBox>());
  }
  if (new_selection.is_range()) {
    // TODO(eval1749): We should call |SetContentChanged()| between anchor and
    // focus.
    SetContentChanged(new_selection.focus_box()->as<TextBox>());
  }
  *root_box->selection_ = new_selection;
  root_box->is_selection_changed_ = true;
}

void BoxEditor::SetViewportSize(RootBox* root_box, const gfx::FloatSize& size) {
  MustBeInTreeRebuild(*root_box);
  if (root_box->viewport_size_ == size)
    return;
  root_box->bounds_ = gfx::FloatRect(size);
  // TODO(eval1749): We don't need to have |RootBox::viewport_size_|. We can
  // use |RootBox::bounds_.size()|.
  root_box->viewport_size_ = size;
  root_box->is_size_changed_ = true;
}

void BoxEditor::WillDestroy(Box* box) {
  box->parent_ = nullptr;
}

}  // namespace visuals
