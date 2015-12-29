// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_
#define EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_

namespace visuals {

// Name, name, type, text
#define FOR_EACH_VISUAL_STYLE_PROPERTY(V)             \
  V(Background, background, Background, "background") \
  V(Border, border, Border, "border")                 \
  V(Color, color, FloatColor, "color")                \
  V(Display, display, Display, "display")             \
  V(Height, height, Height, "height")                 \
  V(Margin, margin, Margin, "margin")                 \
  V(Padding, padding, Padding, "padding")             \
  V(Width, width, Width, "width")

class Background;
class Border;
enum class Display;
class FloatColor;
class Height;
class Padding;
class Margin;
class Width;

enum class StylePropertyId;

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_PROPERTIES_FORWARD_H_
