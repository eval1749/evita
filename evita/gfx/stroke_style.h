// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_stroke_style_h)
#define INCLUDE_evita_gfx_stroke_style_h

#include <vector>

#include "common/win/scoped_comptr.h"
#include "evita/gfx/forward.h"

namespace gfx {

enum class DashStyle {
  Solid = D2D1_DASH_STYLE_SOLID,
  Dash = D2D1_DASH_STYLE_DASH,
  Dot = D2D1_DASH_STYLE_DOT,
  DashDot = D2D1_DASH_STYLE_DASH_DOT,
  DashDotDot = D2D1_DASH_STYLE_DASH_DOT_DOT,
  Custom = D2D1_DASH_STYLE_CUSTOM,
};

class StrokeStyle {
  private: std::vector<float> dashes_;
  private: common::ComPtr<ID2D1StrokeStyle> platform_style_;
  private: D2D1_STROKE_STYLE_PROPERTIES properties_;

  public: StrokeStyle();
  public: ~StrokeStyle();

  public: operator ID2D1StrokeStyle*() const;

  public: bool is_realized() const { return platform_style_; }
  public: void set_dashes(const std::vector<float>& dashes);
  public: void set_dash_offset(float offset);
  public: void set_dash_style(DashStyle dash_style);

  public: void Realize();

  DISALLOW_COPY_AND_ASSIGN(StrokeStyle);
};

}  // namespace gfx


#endif //!defined(INCLUDE_evita_gfx_stroke_style_h)
