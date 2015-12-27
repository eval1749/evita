// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_
#define EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_

namespace visuals {

#define FOR_EACH_VISUAL_STYLE_PROPERTY(V)             \
  V(Background, background, Background, "background") \
  V(Border, border, Border, "border")                 \
  V(Color, color, FloatColor, "color")                \
  V(Margin, margin, Margin, "margin")                 \
  V(Padding, padding, Padding, "padding")

class Background;
class Border;
class FloatColor;
class Padding;
class Margin;

enum class StylePropertyId;

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_
