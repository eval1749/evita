// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "joana/base/graphs/graph_editor.h"
#include "joana/base/graphs/graph_sorter.h"
#include "joana/base/graphs/graph_test_support.h"

namespace joana {

using testing::Block;
using testing::Function;

//////////////////////////////////////////////////////////////////////
//
// GraphTest
//
class GraphTest : public testing::GraphTestBase {
 protected:
  GraphTest() = default;
  ~GraphTest() override = default;

  // ::testing::Test
  void SetUp() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(GraphTest);
};

// Use diamond graph for test cases.
//      1
//     / \
//     2  3
//     \  /
//      4
void GraphTest::SetUp() {
  MakeDiamondGraph();
}

// Test cases...

TEST_F(GraphTest, AddEdge) {
  std::ostringstream ostream;
  ostream << function();
  EXPECT_EQ(
      "{id: 1, predecessors: {}, successors: {B2, B3}}\n"
      "{id: 2, predecessors: {B1}, successors: {B4}}\n"
      "{id: 3, predecessors: {B1}, successors: {B4}}\n"
      "{id: 4, predecessors: {B2, B3}, successors: {}}\n",
      function().ToString());
  auto& block1 = block_at(0);
  auto& block2 = block_at(1);
  auto& block3 = block_at(2);
  auto& block4 = block_at(3);

  EXPECT_TRUE(function().HasEdge(&block1, &block2));
  EXPECT_TRUE(function().HasEdge(&block1, &block3));
  EXPECT_TRUE(function().HasEdge(&block2, &block4));
  EXPECT_TRUE(function().HasEdge(&block3, &block4));

  EXPECT_FALSE(function().HasEdge(&block2, &block1));
  EXPECT_FALSE(function().HasEdge(&block3, &block1));

  EXPECT_FALSE(block1.HasPredecessor());
  EXPECT_TRUE(block1.HasSuccessor());
  EXPECT_FALSE(block1.HasMoreThanOnePredecessor());
  EXPECT_TRUE(block1.HasMoreThanOneSuccessor());
  EXPECT_TRUE(block2.HasPredecessor());
  EXPECT_TRUE(block2.HasSuccessor());
  EXPECT_FALSE(block2.HasMoreThanOnePredecessor());
  EXPECT_FALSE(block2.HasMoreThanOneSuccessor());

  EXPECT_TRUE(block3.HasPredecessor());
  EXPECT_TRUE(block3.HasSuccessor());
  EXPECT_FALSE(block3.HasMoreThanOnePredecessor());
  EXPECT_FALSE(block3.HasMoreThanOneSuccessor());

  EXPECT_TRUE(block4.HasPredecessor());
  EXPECT_FALSE(block4.HasSuccessor());
  EXPECT_TRUE(block4.HasMoreThanOnePredecessor());
  EXPECT_FALSE(block4.HasMoreThanOneSuccessor());
}

TEST_F(GraphTest, InsertNode) {
  Function::Editor editor;

  auto& block2 = block_at(1);
  auto& block4 = block_at(3);

  // Move |block2| before |block4|.
  editor.RemoveNode(&function(), &block2);
  editor.InsertNode(&function(), &block2, &block4);
  EXPECT_EQ(
      "{id: 1, predecessors: {}, successors: {B2, B3}}\n"
      "{id: 3, predecessors: {B1}, successors: {B4}}\n"
      "{id: 2, predecessors: {B1}, successors: {B4}}\n"
      "{id: 4, predecessors: {B2, B3}, successors: {}}\n",
      function().ToString());
}

TEST_F(GraphTest, OrderedList) {
  EXPECT_EQ("[B1, B2, B4, B3]",
            ToString(Function::Sorter::SortByPreOrder(function())));
  EXPECT_EQ("[B4, B2, B3, B1]",
            ToString(Function::Sorter::SortByPostOrder(function())));
  EXPECT_EQ("[B3, B4, B2, B1]",
            ToString(Function::Sorter::SortByReversePreOrder(function())));
  EXPECT_EQ("[B1, B3, B2, B4]",
            ToString(Function::Sorter::SortByReversePostOrder(function())));
}

TEST_F(GraphTest, RemoveEdge) {
  Function::Editor editor;

  auto& block1 = block_at(0);
  auto& block2 = block_at(1);
  auto& block3 = block_at(2);
  auto& block4 = block_at(3);
  editor.RemoveEdge(&function(), &block1, &block2);
  editor.RemoveEdge(&function(), &block2, &block4);
  EXPECT_EQ(
      "{id: 1, predecessors: {}, successors: {B3}}\n"
      "{id: 2, predecessors: {}, successors: {}}\n"
      "{id: 3, predecessors: {B1}, successors: {B4}}\n"
      "{id: 4, predecessors: {B3}, successors: {}}\n",
      function().ToString());

  EXPECT_FALSE(block1.HasPredecessor());
  EXPECT_TRUE(block1.HasSuccessor());
  EXPECT_FALSE(block1.HasMoreThanOnePredecessor());

  EXPECT_FALSE(block2.HasPredecessor());
  EXPECT_FALSE(block2.HasSuccessor());
  EXPECT_FALSE(block2.HasMoreThanOnePredecessor());

  EXPECT_TRUE(block3.HasPredecessor());
  EXPECT_TRUE(block3.HasSuccessor());
  EXPECT_FALSE(block3.HasMoreThanOnePredecessor());

  EXPECT_TRUE(block4.HasPredecessor());
  EXPECT_FALSE(block4.HasSuccessor());
  EXPECT_FALSE(block4.HasMoreThanOnePredecessor());
}

}  // namespace joana
