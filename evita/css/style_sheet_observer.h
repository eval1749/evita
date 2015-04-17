// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style__sheet_observer_h)
#define INCLUDE_evita_css_style__sheet_observer_h

#include "base/macros.h"

namespace css {

class StyleRule;

class StyleSheetObserver {
  public: StyleSheetObserver();
  public: virtual ~StyleSheetObserver();

  public: virtual void DidAddRule(const StyleRule* rule) = 0;
  public: virtual void DidRemoveRule(const StyleRule* rule) = 0;

  DISALLOW_COPY_AND_ASSIGN(StyleSheetObserver);
};

}  // namespace css

#endif //!defined(INCLUDE_evita_css_style__sheet_observer_h)
