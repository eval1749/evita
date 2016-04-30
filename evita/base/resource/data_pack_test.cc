// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/resource/data_pack.h"

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

const char kSamplePakContents[] = {
    0x04, 0x00, 0x00, 0x00,               // header(version
    0x04, 0x00, 0x00, 0x00,               //        no. entries
    0x01,                                 //        encoding)
    0x01, 0x00, 0x27, 0x00, 0x00, 0x00,   // index entry 1
    0x04, 0x00, 0x27, 0x00, 0x00, 0x00,   // index entry 4
    0x06, 0x00, 0x33, 0x00, 0x00, 0x00,   // index entry 6
    0x0a, 0x00, 0x3f, 0x00, 0x00, 0x00,   // index entry 10
    0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,   // extra entry for the size of last
    't', 'h', 'i', 's', ' ', 'i', 's', ' ', 'i', 'd', ' ', '4',
    't', 'h', 'i', 's', ' ', 'i', 's', ' ', 'i', 'd', ' ', '6'
};

const size_t kSamplePakSize = sizeof(kSamplePakContents);

class DataPackTest : public testing::Test{
 protected:
  DataPackTest() = default;
  ~DataPackTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(DataPackTest);
};

TEST_F(DataPackTest, LoadFromFile) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  auto data_path = dir.path().Append(FILE_PATH_LITERAL("sample.pak"));

  ASSERT_EQ(base::WriteFile(data_path, kSamplePakContents, kSamplePakSize),
            static_cast<int>(kSamplePakSize));

  DataPack pack;
  ASSERT_TRUE(pack.LoadFromPath(data_path));
  EXPECT_EQ("this is id 4", pack.GetStringPiece(4));
  EXPECT_EQ("this is id 6", pack.GetStringPiece(6));
  EXPECT_EQ(0u, pack.GetStringPiece(1).length())
    << "An empty entry";
  EXPECT_EQ(0u, pack.GetStringPiece(10).length())
    << "An empty entry";
  EXPECT_EQ(base::StringPiece(), pack.GetStringPiece(12345))
    << "Empty StringPiece for invalid resource id";
}

}  // namespace base
