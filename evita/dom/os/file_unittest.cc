// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"

namespace {

class OsFileTest : public dom::AbstractDomTest {
  protected: OsFileTest() {
  }
  public: virtual ~OsFileTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(OsFileTest);
};

TEST_F(OsFileTest, OsFile_stat_) {
  domapi::QueryFileStatusCallbackData data;
  data.error_code = 123;
  data.file_size = 456;
  data.is_directory = true;
  data.is_symlink = true;
  data.last_write_time = base::Time::FromJsTime(123456.0);
  data.readonly = false;
  mock_io_delegate()->SetQueryFileStatusCallbackData(data);
  EXPECT_SCRIPT_VALID(
    "var result;"
    "function callback(data) {"
    "   result = data;"
    "}"
    "Os.File.stat_('foo', callback);");
  EXPECT_SCRIPT_EQ("123", "result.errorCode");
  EXPECT_SCRIPT_EQ("true", "result.isDir");
  EXPECT_SCRIPT_EQ("true", "result.isSymLink");
  EXPECT_SCRIPT_EQ("123456", "result.lastModificationDate.valueOf()");
  EXPECT_SCRIPT_EQ("false", "result.readonly");
}

}  // namespace
