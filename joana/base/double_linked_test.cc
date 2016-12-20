// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/containers/adapters.h"
#include "joana/base/double_linked.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

namespace {

class List1;
class List2;

class Node : public DoubleLinked<Node, List1>::NodeBase,
             public DoubleLinked<Node, List2>::NodeBase {
 public:
  explicit Node(std::string value) : value_(value) {}

  Node* next() {
    // |next()| is defined in double-linked-list List1 and List2, so we need
    // to have cast |this| pointer.
    return static_cast<DoubleLinked<Node, List1>::NodeBase*>(this)->next();
  }
  Node* next2() {
    return static_cast<DoubleLinked<Node, List2>::NodeBase*>(this)->next();
  }
  const std::string& value() const { return value_; }

 private:
  std::string value_;
};

template <typename T>
class ListOwner {
 public:
  using List = DoubleLinked<Node, T>;

  typename List::Iterator begin() { return nodes_.begin(); }
  bool empty() const { return nodes_.empty(); }
  typename List::Iterator end() { return nodes_.end(); }
  Node* first_node() { return nodes_.first_node(); }
  Node* last_node() { return nodes_.last_node(); }
  typename List::ReverseIterator rbegin() { return nodes_.rbegin(); }
  typename List::ReverseIterator rend() { return nodes_.rend(); }

  void AppendNode(Node* new_node) { nodes_.AppendNode(new_node); }
  int Count() const { return nodes_.Count(); }
  void InsertAfter(Node* new_node, Node* ref_node) {
    nodes_.InsertAfter(new_node, ref_node);
  }
  void InsertBefore(Node* new_node, Node* ref_node) {
    nodes_.InsertBefore(new_node, ref_node);
  }
  void PrependNode(Node* new_node) { nodes_.PrependNode(new_node); }
  void RemoveAll() { nodes_.RemoveAll(); }
  void RemoveNode(Node* old_node) { nodes_.RemoveNode(old_node); }
  void ReplaceNode(Node* new_node, Node* ref_node) {
    nodes_.ReplaceNode(new_node, ref_node);
  }

  std::string ToString() const {
    std::string result;
    auto separator = "";
    for (auto const node : nodes_) {
      result += separator + node->value();
      separator = ", ";
    }
    return result;
  }

 private:
  DoubleLinked<Node, T> nodes_;
};

class List1 : public ListOwner<List1> {};
class List2 : public ListOwner<List2> {};

}  // namespace

class DoubleLinkedTest : public ::testing::Test {
 protected:
  DoubleLinkedTest() : node_a("A"), node_b("B"), node_c("C"), node_d("D") {}

  void SetUp() override {
    list1.AppendNode(&node_a);
    list1.AppendNode(&node_b);
    list1.AppendNode(&node_c);
    list2.AppendNode(&node_c);
    list2.AppendNode(&node_a);
    list2.AppendNode(&node_b);
  }

  List1 list1;
  List2 list2;
  Node node_a;
  Node node_b;
  Node node_c;
  Node node_d;
};

TEST_F(DoubleLinkedTest, empty) {
  EXPECT_FALSE(list1.empty());
  EXPECT_FALSE(list2.empty());
}

TEST_F(DoubleLinkedTest, first_node) {
  EXPECT_EQ(&node_a, list1.first_node());
}

TEST_F(DoubleLinkedTest, last_node) {
  EXPECT_EQ(&node_c, list1.last_node());
}

TEST_F(DoubleLinkedTest, AppendNode) {
  EXPECT_EQ("A, B, C", list1.ToString());
  EXPECT_EQ(&node_b, node_a.next2());
  EXPECT_EQ("C, A, B", list2.ToString());
  EXPECT_EQ(&node_b, node_a.next2());
}

TEST_F(DoubleLinkedTest, Count) {
  EXPECT_EQ(3, list1.Count());
  EXPECT_EQ(3, list2.Count());
}

TEST_F(DoubleLinkedTest, InsertAfter) {
  list1.InsertAfter(&node_d, &node_b);
  EXPECT_EQ("A, B, D, C", list1.ToString());

  list2.InsertAfter(&node_d, nullptr);
  EXPECT_EQ("D, C, A, B", list2.ToString());
}

TEST_F(DoubleLinkedTest, InsertBefore) {
  list1.InsertBefore(&node_d, &node_b);
  EXPECT_EQ("A, D, B, C", list1.ToString());

  list2.InsertBefore(&node_d, nullptr);
  EXPECT_EQ("C, A, B, D", list2.ToString());
}

TEST_F(DoubleLinkedTest, Iterator) {
  std::string result;
  for (auto it = list1.begin(); it != list1.end(); ++it)
    result += it->value();
  EXPECT_EQ("ABC", result);
}

TEST_F(DoubleLinkedTest, IteratorCategory) {
  // |std::vector<T>(begin, end)| constructor requires |iterator_category| for
  // parameters.
  std::vector<Node*> nodes(list1.begin(), list1.end());
  std::string result;
  for (auto node : nodes)
    result += node->value();
  EXPECT_EQ("ABC", result);
}

TEST_F(DoubleLinkedTest, PrependNode) {
  list1.PrependNode(&node_d);
  EXPECT_EQ("D, A, B, C", list1.ToString());
}

TEST_F(DoubleLinkedTest, RemoveAll) {
  list1.RemoveAll();
  EXPECT_TRUE(list1.empty());
}

TEST_F(DoubleLinkedTest, RemoveNode) {
  list1.RemoveNode(&node_a);
  EXPECT_EQ(2, list1.Count());

  list1.RemoveNode(&node_b);
  EXPECT_EQ(1, list1.Count());

  list1.RemoveNode(&node_c);
  EXPECT_EQ(0, list1.Count());
  EXPECT_TRUE(list1.empty());

  list2.RemoveNode(&node_a);
  EXPECT_EQ(2, list2.Count());
}

TEST_F(DoubleLinkedTest, ReplaceNode) {
  Node node_x("X");
  list1.ReplaceNode(&node_x, &node_a);
  EXPECT_EQ("X, B, C", list1.ToString());

  list1.ReplaceNode(&node_a, &node_b);
  EXPECT_EQ("X, A, C", list1.ToString());

  list1.ReplaceNode(&node_b, &node_c);
  EXPECT_EQ("X, A, B", list1.ToString());
}

TEST_F(DoubleLinkedTest, ReverseIterator) {
  std::string result;
  for (auto it = list1.rbegin(); it != list1.rend(); ++it)
    result += it->value();
  EXPECT_EQ("CBA", result);
}

TEST_F(DoubleLinkedTest, Reversed) {
  std::string result;
  for (auto const node : base::Reversed(list1))
    result += node->value();
  EXPECT_EQ("CBA", result);
}

}  // namespace joana
