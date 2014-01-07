// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class FilePathTest : public dom::AbstractDomTest {
  protected: FilePathTest() {
  }
  public: virtual ~FilePathTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(FilePathTest);
};

namespace {
std::string FullPath(const std::string& filename) {
  std::string full_name(MAX_PATH + 1, 0);
  char* file_start = nullptr;
  auto const length = ::GetFullPathNameA(
      filename.c_str(),
      full_name.length(),
      &full_name[0],
      &file_start);
  full_name.resize(length);
  return full_name;
}
} // namespace

TEST_F(FilePathTest, filename) {
  EXPECT_SCRIPT_EQ("bar.txt", "FilePath.filename('/foo/bar.txt')");
}

TEST_F(FilePathTest, fullPath) {
  EXPECT_SCRIPT_EQ(FullPath("foo.txt"), "FilePath.fullPath('foo.txt')");
}

}  // namespace
