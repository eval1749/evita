// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class VisualsTest : public AbstractDomTest {
 public:
  ~VisualsTest() override = default;

 protected:
  VisualsTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(VisualsTest);
};

TEST_F(VisualsTest, CSSStyleSheet) {
  RunFile({"visuals", "css_style_sheet_test.js"});
}

TEST_F(VisualsTest, DOMTokenList) {
  RunFile({"visuals", "dom_token_list_test.js"});
}

TEST_F(VisualsTest, Node) {
  RunFile({"visuals", "node_test.js"});
}

}  // namespace dom
