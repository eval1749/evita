// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/macros.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::_;

class FilePathTest : public dom::AbstractDomTest {
 public:
  ~FilePathTest() override = default;

 protected:
  FilePathTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(FilePathTest);
};

namespace {
std::string FullPath(const std::string& file_name) {
  std::string full_name(MAX_PATH + 1, 0);
  char* file_start = nullptr;
  auto const length = ::GetFullPathNameA(file_name.c_str(),
                                         static_cast<DWORD>(full_name.length()),
                                         &full_name[0], &file_start);
  full_name.resize(length);
  return full_name;
}
}  // namespace

TEST_F(FilePathTest, basename) {
  EXPECT_SCRIPT_EQ("bar.txt", "FilePath.basename('/foo/bar.txt')");
}

TEST_F(FilePathTest, fullPath) {
  EXPECT_SCRIPT_EQ(FullPath("foo.txt"), "FilePath.fullPath('foo.txt')");
}

TEST_F(FilePathTest, isValidFileName) {
  EXPECT_SCRIPT_TRUE("FilePath.isValidFileName('c:/foo/bar.cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('c:/foo:/bar.cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('c:/foo/<bar>.cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('c:/foo/*.cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('c:/foo/bar.?')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('c:/foo/\"bar\".cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('/foo/bar.cc')");
  EXPECT_SCRIPT_FALSE("FilePath.isValidFileName('foo/bar.cc')");
}

}  // namespace
