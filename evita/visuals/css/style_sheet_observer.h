// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_STYLE_SHEET_OBSERVER_H_
#define EVITA_VISUALS_CSS_STYLE_SHEET_OBSERVER_H_

#include "base/macros.h"

namespace visuals {
namespace css {

class Rule;

//////////////////////////////////////////////////////////////////////
//
// StyleSheetObserver
//
class StyleSheetObserver {
 public:
  virtual ~StyleSheetObserver();

  virtual void DidAddRule(const Rule& rule);
  virtual void DidRemoveRule(const Rule& rule);

 protected:
  StyleSheetObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleSheetObserver);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_STYLE_SHEET_OBSERVER_H_
