// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <ostream>

#include "evita/paint/scroll_bar_painter.h"

#include "base/logging.h"
#include "evita/dom/public/scroll_bar_state.h"
#include "evita/gfx/color_f.h"
#include "evita/visuals/css/float_color.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/geometry/affine_transformer.h"

namespace paint {

using AffineTransformer = visuals::AffineTransformer;
using BeginClipDisplayItem = visuals::BeginClipDisplayItem;
using BeginTransformDisplayItem = visuals::BeginTransformDisplayItem;
using ClearDisplayItem = visuals::ClearDisplayItem;
using DrawLineDisplayItem = visuals::DrawLineDisplayItem;
using DisplayItemListBuilder = visuals::DisplayItemListBuilder;
using EndClipDisplayItem = visuals::EndClipDisplayItem;
using EndTransformDisplayItem = visuals::EndTransformDisplayItem;
using FillRectDisplayItem = visuals::FillRectDisplayItem;
using FloatColor = visuals::FloatColor;
using FloatPoint = visuals::FloatPoint;
using FloatSize = visuals::FloatSize;
using FloatRect = visuals::FloatRect;
using ScrollBarState = domapi::ScrollBarState;

namespace {

float AlphaFor(ScrollBarState state) {
  switch (state) {
    case ScrollBarState::Active:
      return 0.6f;
    case ScrollBarState::Disabled:
      return 0.3f;
    case ScrollBarState::Hovered:
      return 0.8f;
    case ScrollBarState::Normal:
      return 0.5f;
    case ScrollBarState::Pressed:
      return 1.0f;
  }
  NOTREACHED();
  return 1.0f;
}

FloatColor BackgroundColor() {
  const auto& color = gfx::sysColor(COLOR_WINDOW);
  return FloatColor(color.red(), color.green(), color.blue());
}

void BeginPaint(DisplayItemListBuilder* builder, const FloatRect& bounds) {
  AffineTransformer transformer;
  transformer.Translate(bounds.origin());
  builder->AddNew<BeginTransformDisplayItem>(transformer.matrix());
  builder->AddNew<BeginClipDisplayItem>(FloatRect(bounds.size()));
  builder->AddNew<ClearDisplayItem>(BackgroundColor());
}

FloatColor ButtonBackgroundColorFor(ScrollBarState state) {
  const auto& color = gfx::sysColor(COLOR_BTNFACE);
  return FloatColor(color.red(), color.green(), color.blue(), AlphaFor(state));
}

FloatColor ButtonColorFor(ScrollBarState state) {
  const auto& color = gfx::sysColor(COLOR_BTNTEXT);
  return FloatColor(color.red(), color.green(), color.blue(), AlphaFor(state));
}

void EndPaint(DisplayItemListBuilder* builder) {
  builder->AddNew<EndClipDisplayItem>();
  builder->AddNew<EndTransformDisplayItem>();
}

FloatColor ThumbColorFor(ScrollBarState state) {
  const auto& color = gfx::sysColor(COLOR_BTNSHADOW);
  return FloatColor(color.red(), color.green(), color.blue(), AlphaFor(state));
}

FloatColor TrackColorFor(ScrollBarState state) {
  const auto& color = gfx::sysColor(COLOR_BTNFACE);
  return FloatColor(color.red(), color.green(), color.blue());
}

void PaintButton(DisplayItemListBuilder* builder,
                 const FloatRect& bounds,
                 const std::array<float, 4> factors,
                 ScrollBarState state) {
  BeginPaint(builder, bounds);

  auto const center_x = bounds.width() / 2;
  auto const center_y = bounds.height() / 2;
  auto const wing_size = ::floor(bounds.width() / 4);

  builder->AddNew<FillRectDisplayItem>(FloatRect(bounds.size()),
                                       ButtonBackgroundColorFor(state));

  auto const pen_width = std::ceil(bounds.height() / 10);
  const auto& color = ButtonColorFor(state);

#if 0
  builder->AddNew<BeginPathDisplayItem>(
      FloatPoint(center_x + factors[0] * wing_size,
                 center_y + factors[1] * wing_size));
  builder->AddNew<PathLineDisplayItem>(FloatPoint(center_x, center_y));
  builder->AddNew<PathLineDisplayItem>(
      FloatPoint(center_x + factors[2] * wing_size,
                 center_y + factors[3] * wing_size));
  builder->AddNew<OpenPathDisplayItem>();

  builder->AddNew<DrawPathDisplayItem>(color, pen_width, LineCapStyle::Flat,
                                       LineJoinStyle::Miter);
  builder->AddNew<EndPathDisplayItem>();
#else
  builder->AddNew<DrawLineDisplayItem>(
      FloatPoint(center_x + factors[0] * wing_size,
                 center_y + factors[1] * wing_size),
      FloatPoint(center_x, center_y), color, pen_width);
  builder->AddNew<DrawLineDisplayItem>(
      FloatPoint(center_x, center_y),
      FloatPoint(center_x + factors[2] * wing_size,
                 center_y + factors[3] * wing_size),
      color, pen_width);
#endif
  EndPaint(builder);
}

void PaintTrack(DisplayItemListBuilder* builder,
                const FloatRect& bounds,
                ScrollBarState state) {
  BeginPaint(builder, bounds);
  builder->AddNew<FillRectDisplayItem>(FloatRect(bounds.size()),
                                       TrackColorFor(state));
  EndPaint(builder);
}

void PaintThumb(DisplayItemListBuilder* builder,
                const FloatRect& bounds,
                ScrollBarState state) {
  BeginPaint(builder, bounds);
  builder->AddNew<FillRectDisplayItem>(FloatRect(bounds.size()),
                                       ThumbColorFor(state));
  EndPaint(builder);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBarPainter
//
ScrollBarPainter::ScrollBarPainter() {}
ScrollBarPainter::~ScrollBarPainter() {}

void ScrollBarPainter::PaintBottomButton(DisplayItemListBuilder* builder,
                                         const FloatRect& bounds,
                                         ScrollBarState state) {
  std::array<float, 4> factors{-1, -1, 1, -1};
  PaintButton(builder, bounds, factors, state);
}

void ScrollBarPainter::PaintBottomTrack(DisplayItemListBuilder* builder,
                                        const FloatRect& bounds,
                                        ScrollBarState state) {
  PaintTrack(builder, bounds, state);
}

void ScrollBarPainter::PaintHorizontalThumb(DisplayItemListBuilder* builder,
                                            const FloatRect& bounds,
                                            ScrollBarState state) {
  PaintThumb(builder, bounds, state);
}

void ScrollBarPainter::PaintLeftButton(DisplayItemListBuilder* builder,
                                       const FloatRect& bounds,
                                       ScrollBarState state) {
  std::array<float, 4> factors{1, -1, 1, 1};
  PaintButton(builder, bounds, factors, state);
}

void ScrollBarPainter::PaintLeftTrack(DisplayItemListBuilder* builder,
                                      const FloatRect& bounds,
                                      ScrollBarState state) {
  PaintTrack(builder, bounds, state);
}

void ScrollBarPainter::PaintRightButton(DisplayItemListBuilder* builder,
                                        const FloatRect& bounds,
                                        ScrollBarState state) {
  std::array<float, 4> factors{-1, -1, -1, 1};
  PaintButton(builder, bounds, factors, state);
}

void ScrollBarPainter::PaintRightTrack(DisplayItemListBuilder* builder,
                                       const FloatRect& bounds,
                                       ScrollBarState state) {
  PaintTrack(builder, bounds, state);
}

void ScrollBarPainter::PaintTopButton(DisplayItemListBuilder* builder,
                                      const FloatRect& bounds,
                                      ScrollBarState state) {
  std::array<float, 4> factors{-1, 1, 1, 1};
  PaintButton(builder, bounds, factors, state);
}

void ScrollBarPainter::PaintTopTrack(DisplayItemListBuilder* builder,
                                     const FloatRect& bounds,
                                     ScrollBarState state) {
  PaintTrack(builder, bounds, state);
}

void ScrollBarPainter::PaintVerticalThumb(DisplayItemListBuilder* builder,
                                          const FloatRect& bounds,
                                          ScrollBarState state) {
  PaintThumb(builder, bounds, state);
}

}  // namespace paint
