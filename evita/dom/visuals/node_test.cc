// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class NodeTest : public AbstractDomTest {
 public:
  ~NodeTest() override = default;

 protected:
  NodeTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

TEST_F(NodeTest, all) {
  RunFile({"visuals", "node_test.js"});
}

}  // namespace dom
