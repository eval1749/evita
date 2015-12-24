// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class OrderedSetTest : public AbstractDomTest {
 public:
  ~OrderedSetTest() override = default;

 protected:
  OrderedSetTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(OrderedSetTest);
};

TEST_F(OrderedSetTest, all) {
  RunFile({"base", "ordered_set_test.js"});
}

}  // namespace dom
