// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_CHANGE_OBSERVER_H_
#define EVITA_VISUALS_STYLE_STYLE_CHANGE_OBSERVER_H_

#include "base/macros.h"

namespace visuals {

namespace css {
class Style;
}

class Element;

//////////////////////////////////////////////////////////////////////
//
// StyleChangeObserver
//
class StyleChangeObserver {
 public:
  virtual ~StyleChangeObserver();

  virtual void DidClearStyleCache();
  virtual void DidRemoveStyleCache(const Element& element,
                                   const css::Style& old_style);

 protected:
  StyleChangeObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleChangeObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_CHANGE_OBSERVER_H_
