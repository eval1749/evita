// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_SHEET_OBSERVER_H_
#define EVITA_CSS_STYLE_SHEET_OBSERVER_H_

#include "base/macros.h"

namespace css {

class Rule;

//////////////////////////////////////////////////////////////////////
//
// StyleSheetObserver
//
class StyleSheetObserver {
 public:
  virtual ~StyleSheetObserver();

  virtual void DidInsertRule(const Rule& rule, size_t index);
  virtual void DidRemoveRule(const Rule& rule, size_t index);

 protected:
  StyleSheetObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleSheetObserver);
};

}  // namespace css

#endif  // EVITA_CSS_STYLE_SHEET_OBSERVER_H_
