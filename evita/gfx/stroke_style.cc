// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/stroke_style.h"

#include "base/logging.h"
#include "evita/gfx_base.h"

namespace gfx {

StrokeStyle::StrokeStyle() {
  properties_.startCap = D2D1_CAP_STYLE_FLAT;
  properties_.endCap = D2D1_CAP_STYLE_FLAT;
  properties_.dashCap = D2D1_CAP_STYLE_FLAT;
  properties_.lineJoin = D2D1_LINE_JOIN_MITER;
  properties_.miterLimit = 0.0f;
  properties_.dashStyle = D2D1_DASH_STYLE_SOLID;
  properties_.dashOffset = 0.0f;
}

StrokeStyle::~StrokeStyle() {
}

StrokeStyle::operator ID2D1StrokeStyle*() const {
  DCHECK(is_realized());
  return platform_style_.get();
}

void StrokeStyle::set_cap_style(CapStyle start_style, CapStyle end_style) {
  properties_.startCap = static_cast<D2D1_CAP_STYLE>(start_style);
  properties_.endCap = static_cast<D2D1_CAP_STYLE>(end_style);
}

void StrokeStyle::set_cap_style(CapStyle cap_style) {
  set_cap_style(cap_style, cap_style);
}

void StrokeStyle::set_dashes(const std::vector<float>& dashes) {
  dashes_ = dashes;
}

void StrokeStyle::set_dash_style(DashStyle dash_style) {
  DCHECK(!is_realized());
  properties_.dashStyle = static_cast<D2D1_DASH_STYLE>(dash_style);
}

void StrokeStyle::set_dash_offset(float offset) {
  DCHECK(!is_realized());
  properties_.dashOffset = offset;
}

void StrokeStyle::set_line_join(LineJoin line_join) {
  DCHECK(!is_realized());
  properties_.lineJoin = static_cast<D2D1_LINE_JOIN>(line_join);
}

void StrokeStyle::Realize() {
  DCHECK(!is_realized());
  COM_VERIFY(FactorySet::instance()->d2d1().CreateStrokeStyle(
    properties_,
    dashes_.data(),
    static_cast<UINT>(dashes_.size()),
    &platform_style_));
}

}  // namespace gfx
