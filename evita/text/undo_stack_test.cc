// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "base/strings/utf_string_conversions.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/text/scoped_undo_group.h"

namespace text {

class UndoStackTest : public ::testing::Test {
 public:
  Buffer* buffer() const { return buffer_.get(); }

  void InsertBefore(Offset offset, const char* text) {
    buffer()->InsertBefore(offset, base::ASCIIToUTF16(text));
  }

 protected:
  UndoStackTest() : buffer_(new Buffer()) {}

 private:
  std::unique_ptr<Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(UndoStackTest);
};

TEST_F(UndoStackTest, CanUndo) {
  InsertBefore(Offset(0), "foo");
  EXPECT_TRUE(buffer()->CanUndo());
  EXPECT_FALSE(buffer()->CanRedo());
  buffer()->Undo(Offset(3));
  EXPECT_FALSE(buffer()->CanUndo());
  EXPECT_TRUE(buffer()->CanRedo());
}

TEST_F(UndoStackTest, Clear) {
  InsertBefore(Offset(0), "foo");
  buffer()->ClearUndo();
  EXPECT_FALSE(buffer()->CanUndo()) << "No more undo steps after ClearUndo()";
}

TEST_F(UndoStackTest, Delete) {
  InsertBefore(Offset(0), "foo");
  auto const anchor_revision = buffer()->revision();

  buffer()->Delete(Offset(0), Offset(3));
  buffer()->Undo(Offset(0));
  EXPECT_EQ(Offset(3), buffer()->GetEnd());
  EXPECT_EQ(anchor_revision, buffer()->revision())
      << "Undo delete should make document not modified.";

  buffer()->Redo(Offset(3));
  EXPECT_EQ(Offset(0), buffer()->GetEnd());
  EXPECT_NE(anchor_revision, buffer()->revision());

  buffer()->Undo(Offset(0));
  EXPECT_EQ(anchor_revision, buffer()->revision())
      << "Undo should make document not modified even if after redo.";
}

TEST_F(UndoStackTest, Group) {
  {
    ScopedUndoGroup undo_group(buffer(), L"test");
    InsertBefore(Offset(0), "foo");
    InsertBefore(Offset(1), "bar");
  }
  buffer()->Undo(Offset(4));
  EXPECT_EQ(Offset(0), buffer()->GetEnd());
  EXPECT_EQ(0, buffer()->revision())
      << "Undo should make document not modified even if after redo.";
}

TEST_F(UndoStackTest, Insert) {
  InsertBefore(Offset(0), "foo");
  EXPECT_EQ(Offset(3), buffer()->Undo(Offset(2)))
      << "Move to the last editing position";
  EXPECT_EQ(Offset(3), buffer()->GetEnd())
      << "Undo isn't occured position other than last editing position.";
  EXPECT_EQ(Offset(0), buffer()->Undo(Offset(3)))
      << "Undo at the last editing position";
  EXPECT_EQ(Offset(0), buffer()->GetEnd());
  EXPECT_EQ(0, buffer()->revision())
      << "Undo should make document not modified.";

  buffer()->Redo(Offset(0));
  EXPECT_EQ(Offset(3), buffer()->GetEnd());
  EXPECT_NE(0, buffer()->revision());

  buffer()->Undo(Offset(3));
  EXPECT_EQ(0, buffer()->revision())
      << "Undo should make document not modified.";
}

TEST_F(UndoStackTest, Merge) {
  {
    ScopedUndoGroup undo_group(buffer(), L"test");
    InsertBefore(Offset(0), "foo");
  }
  {
    ScopedUndoGroup undo_group(buffer(), L"test");
    buffer()->InsertBefore(Offset(3), L"bar");
  }
  EXPECT_EQ(Offset(3), buffer()->Undo(Offset(6)));
  EXPECT_EQ(Offset(3), buffer()->GetEnd());
  EXPECT_EQ(Offset(), buffer()->Undo(Offset(3)));
  EXPECT_EQ(0, buffer()->revision())
      << "Undo should make document not modified.";
}

}  // namespace text
