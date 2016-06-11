// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/stroke_style_builder.h"

#include "evita/gfx/canvas.h"

namespace gfx {

using Builder = StrokeStyle::Builder;

Builder::Builder() {
  properties_.startCap = D2D1_CAP_STYLE_FLAT;
  properties_.endCap = D2D1_CAP_STYLE_FLAT;
  properties_.dashCap = D2D1_CAP_STYLE_FLAT;
  properties_.lineJoin = D2D1_LINE_JOIN_MITER;
  properties_.miterLimit = 0.0f;
  properties_.dashStyle = D2D1_DASH_STYLE_SOLID;
  properties_.dashOffset = 0.0f;
  properties_.transformType = D2D1_STROKE_TRANSFORM_TYPE_NORMAL;
}

Builder::~Builder() = default;

StrokeStyle Builder::Build(Canvas* canvas) {
  common::ComPtr<ID2D1Factory1> factory1;
  COM_VERIFY(factory1.QueryFrom(canvas->GetD2D1Factory()));
  common::ComPtr<ID2D1StrokeStyle1> platform_style;
  COM_VERIFY(factory1->CreateStrokeStyle(properties_, dashes_.data(),
                                         static_cast<UINT>(dashes_.size()),
                                         &platform_style));
  return std::move(StrokeStyle(std::move(platform_style)));
}

Builder& Builder::SetCapStyle(CapStyle start_style, CapStyle end_style) {
  properties_.startCap = static_cast<D2D1_CAP_STYLE>(start_style);
  properties_.endCap = static_cast<D2D1_CAP_STYLE>(end_style);
  return *this;
}

Builder& Builder::SetCapStyle(CapStyle cap_style) {
  return SetCapStyle(cap_style, cap_style);
}

Builder& Builder::SetDashes(const std::vector<float>& dashes) {
  dashes_ = dashes;
  return *this;
}

Builder& Builder::SetDashStyle(DashStyle dash_style) {
  properties_.dashStyle = static_cast<D2D1_DASH_STYLE>(dash_style);
  return *this;
}

Builder& Builder::SetDashOffset(float offset) {
  properties_.dashOffset = offset;
  return *this;
}

Builder& Builder::SetLineJoin(LineJoin line_join) {
  properties_.lineJoin = static_cast<D2D1_LINE_JOIN>(line_join);
  return *this;
}

Builder& Builder::SetStrokeTransform(StrokeTransform transform) {
  properties_.transformType =
      static_cast<D2D1_STROKE_TRANSFORM_TYPE>(transform);
  return *this;
}

}  // namespace gfx
