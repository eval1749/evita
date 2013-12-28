// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <ostream>
#include <vector>

#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "common/tree/node.h"
#include "gtest/gtest.h"

namespace {

class Node : public common::tree::Node<Node> {
  private: int value_;

  public: Node(int value) : value_(value) {
  }

  public: int value() const { return value_; }
  public: void set_value(int value) { value_ = value; }
};

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  ostream << "node" << node.value();
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (node)
    ostream << *node;
  else
    ostream << "null";
  return ostream;
}

std::vector<Node*> GetAncestorsOrSelf(Node* node) {
  std::vector<Node*> nodes;
  for (auto child : common::tree::ancestors_or_self(node)) {
    nodes.push_back(child);
  }
  return std::move(nodes);
}

std::vector<Node*> GetChildNodes(Node* node) {
  std::vector<Node*> nodes;
  for (auto child : node->child_nodes()) {
    nodes.push_back(child);
  }
  return std::move(nodes);
}

std::vector<Node*> GetDescendants(Node* node) {
  std::vector<Node*> nodes;
  for (auto child : common::tree::descendants(node)) {
    nodes.push_back(child);
  }
  return std::move(nodes);
}

std::vector<Node*> GetDescendantsOrSelf(Node* node) {
  std::vector<Node*> nodes;
  for (auto child : common::tree::descendants_or_self(node)) {
    nodes.push_back(child);
  }
  return std::move(nodes);
}

TEST(Node, AncestorsOrSelfOne) {
  Node node1(1);
  std::vector<Node*> expected_value { &node1 };
  EXPECT_EQ(expected_value, GetAncestorsOrSelf(&node1));
}

TEST(Node, AncestorsOrSelfTwo) {
  Node node1(1);
  Node node2(2);
  node1.AppendChild(&node2);
  std::vector<Node*> expected_value { &node2, &node1 };
  EXPECT_EQ(expected_value, GetAncestorsOrSelf(&node2));
}

TEST(Node, AncestorsOrSelfThree) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  std::vector<Node*> expected_value { &node3, &node2, &node1 };
  EXPECT_EQ(expected_value, GetAncestorsOrSelf(&node3));
}

TEST(Node, AppendChild) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.AppendChild(&node2);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node2, node1.last_child());
  EXPECT_EQ(nullptr, node1.next_sibling());
  EXPECT_EQ(nullptr, node1.previous_sibling());

  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  node1.AppendChild(&node3);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node3, node1.last_child());
  EXPECT_EQ(nullptr, node1.next_sibling());
  EXPECT_EQ(nullptr, node1.previous_sibling());

  EXPECT_EQ(node3, node2.next_sibling());
  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(node2, node3.previous_sibling());
}

TEST(Node, ChildNodesZero) {
  Node node1(1);
  std::vector<Node*> expected_value;
  EXPECT_EQ(expected_value, GetChildNodes(&node1));
}

TEST(Node, ChildNodesOne) {
  Node node1(1);
  Node node2(2);

  node1.AppendChild(&node2);
  std::vector<Node*> expected_value = { &node2 };
  EXPECT_EQ(expected_value, GetChildNodes(&node1));
}

TEST(Node, ChildNodesTwo) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  std::vector<Node*> expected_value = { &node2, &node3 };
  EXPECT_EQ(expected_value, GetChildNodes(&node1));
}

TEST(Node, ChildNodesThree) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.AppendChild(&node4);
  std::vector<Node*> expected_value = { &node2, &node3, &node4 };
  EXPECT_EQ(expected_value, GetChildNodes(&node1));
}

TEST(Node, ChildNodesNoChildChild) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);
  Node node5(5);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.AppendChild(&node4);
  node3.AppendChild(&node5);
  std::vector<Node*> expected_value = { &node2, &node3, &node4 };
  EXPECT_EQ(expected_value, GetChildNodes(&node1));
}

TEST(Node, ContainsOne) {
  Node node1(1);
  Node node2(2);
  EXPECT_FALSE(node1.Contains(&node1));
  EXPECT_FALSE(node1.Contains(&node2));
}

TEST(Node, ContainsTwo) {
  Node node1(1);
  Node node2(2);
  node1.AppendChild(&node2);
  EXPECT_TRUE(node1.Contains(&node2));
}

TEST(Node, ContainsThree) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  EXPECT_TRUE(node1.Contains(&node2));
  EXPECT_TRUE(node1.Contains(&node3));
}

TEST(Node, ContainsFour) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  node2.AppendChild(&node4);
  EXPECT_TRUE(node1.Contains(&node2));
  EXPECT_TRUE(node1.Contains(&node3));
  EXPECT_TRUE(node1.Contains(&node4));
}

TEST(Node, DescendantsOne) {
  Node node1(1);
  std::vector<Node*> expected_value { };
  EXPECT_EQ(expected_value, GetDescendants(&node1));
}

TEST(Node, DescendantsTwo) {
  Node node1(1);
  Node node2(2);
  node1.AppendChild(&node2);
  std::vector<Node*> expected_value { &node2 };
  EXPECT_EQ(expected_value, GetDescendants(&node1));
}

TEST(Node, DescendantsThree) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  std::vector<Node*> expected_value { &node2, &node3 };
  EXPECT_EQ(expected_value, GetDescendants(&node1));
}

TEST(Node, DescendantsFour) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  node2.AppendChild(&node4);
  std::vector<Node*> expected_value { &node2, &node3, &node4 };
  EXPECT_EQ(expected_value, GetDescendants(&node1));
}

TEST(Node, DescendantsOrSelfOne) {
  Node node1(1);
  std::vector<Node*> expected_value { &node1 };
  EXPECT_EQ(expected_value, GetDescendantsOrSelf(&node1));
}

TEST(Node, DescendantsOrSelfTwo) {
  Node node1(1);
  Node node2(2);
  node1.AppendChild(&node2);
  std::vector<Node*> expected_value { &node1, &node2 };
  EXPECT_EQ(expected_value, GetDescendantsOrSelf(&node1));
}

TEST(Node, DescendantsOrSelfThree) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  std::vector<Node*> expected_value { &node1, &node2, &node3 };
  EXPECT_EQ(expected_value, GetDescendantsOrSelf(&node1));
}

TEST(Node, DescendantsOrSelfFour) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);
  node1.AppendChild(&node2);
  node2.AppendChild(&node3);
  node2.AppendChild(&node4);
  std::vector<Node*> expected_value { &node1, &node2, &node3, &node4 };
  EXPECT_EQ(expected_value, GetDescendantsOrSelf(&node1));
}

TEST(Node, InsertAfter) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.AppendChild(&node2);
  node1.InsertAfter(&node3, &node2);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node3, node1.last_child());

  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(node3, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(node2, node3.previous_sibling());
}

TEST(Node, InsertBefore) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.AppendChild(&node2);
  node1.InsertBefore(&node3, &node2);

  EXPECT_EQ(node3, node1.first_child());
  EXPECT_EQ(node2, node1.last_child());

  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(node3, node2.previous_sibling());

  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(node2, node3.next_sibling());
  EXPECT_EQ(nullptr, node3.previous_sibling());
}

TEST(Node, PrependChild) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.PrependChild(&node2);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node2, node1.last_child());
  EXPECT_EQ(nullptr, node1.next_sibling());
  EXPECT_EQ(nullptr, node1.previous_sibling());

  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  node1.PrependChild(&node3);

  EXPECT_EQ(node3, node1.first_child());
  EXPECT_EQ(node2, node1.last_child());
  EXPECT_EQ(nullptr, node1.next_sibling());
  EXPECT_EQ(nullptr, node1.previous_sibling());

  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(node3, node2.previous_sibling());

  EXPECT_EQ(node2, node3.next_sibling());
  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(nullptr, node3.previous_sibling());
}

TEST(Node, RemoveChild) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(3);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.AppendChild(&node4);
  node1.RemoveChild(&node3);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node4, node1.last_child());

  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(node4, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(nullptr, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(nullptr, node3.previous_sibling());

  EXPECT_EQ(node1, node4.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(node2, node4.previous_sibling());

  node1.RemoveChild(&node2);

  EXPECT_EQ(node4, node1.first_child());
  EXPECT_EQ(node4, node1.last_child());

  EXPECT_EQ(nullptr, node2.parent_node());
  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(node1, node4.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(nullptr, node4.previous_sibling());

  node1.RemoveChild(&node4);

  EXPECT_EQ(nullptr, node1.first_child());
  EXPECT_EQ(nullptr, node1.last_child());

  EXPECT_EQ(nullptr, node4.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(nullptr, node4.previous_sibling());
}

TEST(Node, ReplaceChildSingle) {
  Node node1(1);
  Node node2(2);
  Node node3(3);

  node1.AppendChild(&node2);
  node1.ReplaceChild(&node3, &node2);

  EXPECT_EQ(node3, node1.first_child());
  EXPECT_EQ(node3, node1.last_child());

  EXPECT_EQ(nullptr, node2.parent_node());
  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(nullptr, node3.previous_sibling());
}

TEST(Node, ReplaceChildFirst) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.ReplaceChild(&node4, &node2);

  EXPECT_EQ(node4, node1.first_child());
  EXPECT_EQ(node3, node1.last_child());

  EXPECT_EQ(nullptr, node2.parent_node());
  EXPECT_EQ(nullptr, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(node1, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(node4, node3.previous_sibling());

  EXPECT_EQ(node1, node4.parent_node());
  EXPECT_EQ(node3, node4.next_sibling());
  EXPECT_EQ(nullptr, node4.previous_sibling());
}

TEST(Node, ReplaceChildLast) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.ReplaceChild(&node4, &node3);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node4, node1.last_child());

  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(node4, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(nullptr, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(nullptr, node3.previous_sibling());

  EXPECT_EQ(node1, node4.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(node2, node4.previous_sibling());
}

TEST(Node, ReplaceChildMiddle) {
  Node node1(1);
  Node node2(2);
  Node node3(3);
  Node node4(4);
  Node node5(5);

  node1.AppendChild(&node2);
  node1.AppendChild(&node3);
  node1.AppendChild(&node4);
  node1.ReplaceChild(&node5, &node3);

  EXPECT_EQ(node2, node1.first_child());
  EXPECT_EQ(node4, node1.last_child());

  EXPECT_EQ(node1, node2.parent_node());
  EXPECT_EQ(node5, node2.next_sibling());
  EXPECT_EQ(nullptr, node2.previous_sibling());

  EXPECT_EQ(nullptr, node3.parent_node());
  EXPECT_EQ(nullptr, node3.next_sibling());
  EXPECT_EQ(nullptr, node3.previous_sibling());

  EXPECT_EQ(node1, node4.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(node5, node4.previous_sibling());

  EXPECT_EQ(node1, node5.parent_node());
  EXPECT_EQ(nullptr, node4.next_sibling());
  EXPECT_EQ(node5, node4.previous_sibling());
}

}  // namespace
