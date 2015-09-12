// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_STROKE_STYLE_H_
#define EVITA_GFX_STROKE_STYLE_H_

#include <vector>

#include "common/win/scoped_comptr.h"
#include "evita/gfx/forward.h"

namespace gfx {

enum class CapStyle {
  Flat = D2D1_CAP_STYLE_FLAT,
  Round = D2D1_CAP_STYLE_ROUND,
  Square = D2D1_CAP_STYLE_SQUARE,
  Triangle = D2D1_CAP_STYLE_TRIANGLE,
};

enum class DashStyle {
  Solid = D2D1_DASH_STYLE_SOLID,
  Dash = D2D1_DASH_STYLE_DASH,
  Dot = D2D1_DASH_STYLE_DOT,
  DashDot = D2D1_DASH_STYLE_DASH_DOT,
  DashDotDot = D2D1_DASH_STYLE_DASH_DOT_DOT,
  Custom = D2D1_DASH_STYLE_CUSTOM,
};

enum class LineJoin {
  Miter = D2D1_LINE_JOIN_MITER,
  Bevel = D2D1_LINE_JOIN_BEVEL,
  Round = D2D1_LINE_JOIN_ROUND,
  MiterOrBevel = D2D1_LINE_JOIN_MITER_OR_BEVEL,
};

class StrokeStyle final {
 public:
  StrokeStyle();
  ~StrokeStyle();

  operator ID2D1StrokeStyle*() const;

  bool is_realized() const { return platform_style_ != nullptr; }
  void set_cap_style(CapStyle start_style, CapStyle end_cap_style);
  void set_cap_style(CapStyle cap_style);
  void set_dashes(const std::vector<float>& dashes);
  void set_dash_offset(float offset);
  void set_dash_style(DashStyle dash_style);
  void set_line_join(LineJoin line_join);

  void Realize();

 private:
  std::vector<float> dashes_;
  common::ComPtr<ID2D1StrokeStyle> platform_style_;
  D2D1_STROKE_STYLE_PROPERTIES properties_;

  DISALLOW_COPY_AND_ASSIGN(StrokeStyle);
};

}  // namespace gfx

#endif  // EVITA_GFX_STROKE_STYLE_H_
