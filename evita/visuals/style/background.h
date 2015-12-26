// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_BACKGROUND_H_
#define EVITA_VISUALS_STYLE_BACKGROUND_H_

#include "evita/visuals/style/float_color.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Background is an implementation of CSS Background and Borders Module
// Level 3.
//
// Supports following CSS properties:
//  background-color: <color>
// Not yet implemented:
//  background-attachment: <attachment> [, <attachment>]*
//  background-clip: <box> [, <box>]*
//  background-image: <bg-image> [, <bg-image>]*
//  background-position: <position>, [, <position>]*
//  background-repeat: <repeat-style> [, <repeat-style>]*
//  background-size: <bg-size> [, <bg-size>]
//
//  <attachment> = scroll | fixed | local
//  <bg-image> = <image> | none
//  <bg-size> = [<length> | <percentage> | auto]{1,2} | cover | contain
//  <box> = border-box | padding-box | content-box
//  <position> ::= [left | center | right | top | bottom | <percentage> |
//      <length>] | [left | center |right | <percentage> | <length>]
//      [top | center | bottom | <percentage> | <length> ]
//      [center | [left | right] [<percentage> | <length>]?] &&
//      [center | [top | bottom] [<percentage> | <length>]?]
//  <repeat-style> = repeat-x | repeat-y |
//      [repeat | space | round | no-repeat]{1, 2}
//
class Background final {
 public:
  explicit Background(const FloatColor& color);
  Background(const Background& other);
  Background();
  ~Background();

  bool operator==(const Background& other) const;
  bool operator!=(const Background& other) const;

  const FloatColor& color() const { return color_; }

 private:
  FloatColor color_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_BACKGROUND_H_
