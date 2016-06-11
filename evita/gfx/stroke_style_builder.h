// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_STROKE_STYLE_BUILDER_H_
#define EVITA_GFX_STROKE_STYLE_BUILDER_H_

#include <vector>

#include "evita/gfx/stroke_style.h"

namespace gfx {

class StrokeStyle::Builder {
 public:
  Builder(const Builder& other) = delete;
  Builder();
  ~Builder();

  Builder operator=(const Builder& other) = delete;

  StrokeStyle Build(Canvas* canvas);
  Builder& SetCapStyle(CapStyle start_style, CapStyle end_cap_style);
  Builder& SetCapStyle(CapStyle cap_style);
  Builder& SetDashes(const std::vector<float>& dashes);
  Builder& SetDashOffset(float offset);
  Builder& SetDashStyle(DashStyle dash_style);
  Builder& SetLineJoin(LineJoin line_join);
  Builder& SetStrokeTransform(StrokeTransform transform);

 private:
  std::vector<float> dashes_;
  D2D1_STROKE_STYLE_PROPERTIES1 properties_;
};

}  // namespace gfx

#endif  // EVITA_GFX_STROKE_STYLE_BUILDER_H_
