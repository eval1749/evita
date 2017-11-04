// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class UnicodeTest : public dom::AbstractDomTest {
 public:
  ~UnicodeTest() override = default;

 protected:
  UnicodeTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UnicodeTest);
};

TEST_F(UnicodeTest, File) {
  RunFile({"unicode", "unicode_test.js"});
}

}  // namespace dom
