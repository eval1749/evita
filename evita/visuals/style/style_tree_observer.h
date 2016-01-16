// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_TREE_OBSERVER_H_
#define EVITA_VISUALS_STYLE_STYLE_TREE_OBSERVER_H_

#include "base/macros.h"

namespace visuals {

namespace css {
class Style;
}

class ElementNode;

//////////////////////////////////////////////////////////////////////
//
// StyleTreeObserver
//
class StyleTreeObserver {
 public:
  virtual ~StyleTreeObserver();

  virtual void DidChangeComputedStyle(const ElementNode& element,
                                      const css::Style& old_style);

 protected:
  StyleTreeObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleTreeObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_TREE_OBSERVER_H_
