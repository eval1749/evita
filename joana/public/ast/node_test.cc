// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/public/ast/node.h"

#include "base/macros.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/punctuator.h"
#include "joana/public/ast/statements.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_range.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace ast {

//
// NodeTest
//
class NodeTest : public ::testing::Test {
 protected:
  NodeTest();
  ~NodeTest() override = default;

  NodeFactory& factory() { return node_factory_; }
  SourceCodeRange range() const { return source_code_->range(); }

  BlockStatement& NewBlockStatement();
  EmptyStatement& NewEmptyStatement();

 private:
  Zone zone_;
  NodeFactory node_factory_;
  SourceCode::Factory source_code_factory_;
  const SourceCode* const source_code_;

  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

NodeTest::NodeTest()
    : zone_("NodeTest"),
      node_factory_(&zone_),
      source_code_factory_(&zone_),
      source_code_(&source_code_factory_.New(base::FilePath(),
                                             base::StringPiece16({}, 0))) {}

BlockStatement& NodeTest::NewBlockStatement() {
  return factory().NewBlockStatement(
      factory().NewPunctuator(range(), PunctuatorKind::LeftBrace));
}

EmptyStatement& NodeTest::NewEmptyStatement() {
  return factory().NewEmptyStatement(
      factory().NewPunctuator(range(), PunctuatorKind::SemiColon));
}

TEST_F(NodeTest, NodeContains) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& block2 = NewBlockStatement();
  NodeEditor().AppendChild(&module, &block2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&block2, &statement3);

  EXPECT_TRUE(module.Contains(statement3));
  EXPECT_FALSE(block2.Contains(statement3));
}

TEST_F(NodeTest, NodeEditorAppendChild) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement1, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement1));

  auto& statement2 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement2);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement2, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(statement2, NodeTraversal::NextSiblingOf(statement1));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement2));
  EXPECT_EQ(statement1, NodeTraversal::PreviousSiblingOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement2));
}

TEST_F(NodeTest, NodeEditorInsertBefore) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().InsertBefore(&module, &statement1, nullptr);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement1, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));

  auto& statement2 = NewEmptyStatement();
  NodeEditor().InsertBefore(&module, &statement2, &statement1);
  EXPECT_EQ(statement2, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement1, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(statement2, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement1));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement2));
  EXPECT_EQ(statement1, NodeTraversal::NextSiblingOf(statement2));
}

TEST_F(NodeTest, NodeEditorRemoveChild) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& statement2 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement3);

  NodeEditor().RemoveChild(&module, &statement2);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement3, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(statement3, NodeTraversal::NextSiblingOf(statement1));

  EXPECT_EQ(nullptr, NodeTraversal::ParentOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement2));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement3));
  EXPECT_EQ(statement1, NodeTraversal::PreviousSiblingOf(statement3));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement3));
}

TEST_F(NodeTest, NodeEditorRemoveNode) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& statement2 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement3);

  NodeEditor().RemoveNode(&statement2);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement3, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(statement3, NodeTraversal::NextSiblingOf(statement1));

  EXPECT_EQ(nullptr, NodeTraversal::ParentOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement2));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement3));
  EXPECT_EQ(statement1, NodeTraversal::PreviousSiblingOf(statement3));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement3));
}

TEST_F(NodeTest, NodeEditorReplaceChild) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& statement2 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement3);

  auto& statement4 = NewEmptyStatement();
  NodeEditor().ReplaceChild(&module, &statement4, &statement2);
  EXPECT_EQ(statement1, NodeTraversal::FirstChildOf(module));
  EXPECT_EQ(statement3, NodeTraversal::LastChildOf(module));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement1));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement1));
  EXPECT_EQ(statement4, NodeTraversal::NextSiblingOf(statement1));

  EXPECT_EQ(nullptr, NodeTraversal::ParentOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::PreviousSiblingOf(statement2));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement2));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement3));
  EXPECT_EQ(statement4, NodeTraversal::PreviousSiblingOf(statement3));
  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(statement3));

  EXPECT_EQ(module, NodeTraversal::ParentOf(statement4));
  EXPECT_EQ(statement1, NodeTraversal::PreviousSiblingOf(statement4));
  EXPECT_EQ(statement3, NodeTraversal::NextSiblingOf(statement4));
}

TEST_F(NodeTest, NodeTraversalAncestorsOf) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& block2 = NewBlockStatement();
  NodeEditor().AppendChild(&module, &block2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&block2, &statement3);

  std::vector<const Node*> result;
  for (const auto& node : NodeTraversal::AncestorsOf(statement3))
    result.push_back(&node);

  EXPECT_EQ((std::vector<const Node*>{&block2, &module}), result);
}

TEST_F(NodeTest, NodeTraversalInclusiveAncestorsOf) {
  auto& module = factory().NewModule(range());
  auto& statement1 = NewEmptyStatement();
  NodeEditor().AppendChild(&module, &statement1);
  auto& block2 = NewBlockStatement();
  NodeEditor().AppendChild(&module, &block2);
  auto& statement3 = NewEmptyStatement();
  NodeEditor().AppendChild(&block2, &statement3);

  std::vector<const Node*> result;
  for (const auto& node : NodeTraversal::InclusiveAncestorsOf(statement3))
    result.push_back(&node);

  EXPECT_EQ((std::vector<const Node*>{&statement3, &block2, &module}), result);
}

}  // namespace ast
}  // namespace joana