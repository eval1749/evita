// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_VISUALS_CSS_STYLE_H_
#define EVITA_DOM_VISUALS_CSS_STYLE_H_

#include <memory>

#include "base/macros.h"
#include "evita/ginx/v8.h"

namespace visuals {
namespace css {
class Style;
}
}

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// CSSStyle
//
class CSSStyle final {
 public:
  using CssStyle = visuals::css::Style;

  static std::unique_ptr<CssStyle> ConvertFromV8(v8::Local<v8::Context> context,
                                                 v8::Local<v8::Map> raw_style);

  static v8::Local<v8::Map> ConvertToV8(v8::Local<v8::Context> context,
                                        const CssStyle& style);

 private:
  DISALLOW_COPY_AND_ASSIGN(CSSStyle);
};

}  // namespace dom

#endif  // EVITA_DOM_VISUALS_CSS_STYLE_H_
