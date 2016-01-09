// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class MutationObserverTest : public AbstractDomTest {
 protected:
  MutationObserverTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(MutationObserverTest);
};

TEST_F(MutationObserverTest, ctor) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var range = new TextRange(doc);"
      "range.text = 'foo bar baz';"
      "function callback(records, observer) {"
      "}"
      "var observer = new MutationObserver(callback);"
      "range.text = ''");
}

}  // namespace dom
