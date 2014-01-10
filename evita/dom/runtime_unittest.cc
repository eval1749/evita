// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"

namespace {

class RuntimeTest : public dom::AbstractDomTest {
  protected: RuntimeTest() {
  }
  public: virtual ~RuntimeTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(RuntimeTest);
};

TEST_F(RuntimeTest, localizeText) {
  EXPECT_SCRIPT_EQ("This is a pen",
      "localizeText('This is a __item__', {item: 'pen'})");
}

}  // namespace
