// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class CSSStyleSheetTest : public AbstractDomTest {
 public:
  ~CSSStyleSheetTest() override = default;

 protected:
  CSSStyleSheetTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(CSSStyleSheetTest);
};

TEST_F(CSSStyleSheetTest, all) {
  RunFile({"visuals", "css_style_sheet_test.js"});
}

}  // namespace dom
