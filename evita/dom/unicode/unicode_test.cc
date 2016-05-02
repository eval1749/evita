// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class UnicodTest : public dom::AbstractDomTest {
 public:
  ~UnicodTest() override = default;

 protected:
  UnicodTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UnicodTest);
};

TEST_F(UnicodTest, File) {
  RunFile({"unicode", "unicode_test.js"});
}

}  // namespace dom
