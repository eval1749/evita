// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/node.h"

#include "base/files/file_path.h"
#include "joana/public/ast/container_node.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/escaped_string_piece.h"
#include "joana/public/source_code.h"

namespace joana {
namespace ast {

Node::Node(const SourceCodeRange& range) : range_(range) {}
Node::~Node() = default;

bool Node::operator==(const Node& other) const {
  return this == &other;
}

bool Node::operator==(const Node* other) const {
  return this == other;
}

bool Node::operator!=(const Node& other) const {
  return this != &other;
}

bool Node::operator!=(const Node* other) const {
  return this != other;
}

Node* Node::first_child() const {
  return nullptr;
}
Node* Node::last_child() const {
  return nullptr;
}

bool Node::Contains(const Node& other) const {
  return this == &other || other.IsDescendantOf(*this);
}

bool Node::IsDescendantOf(const Node& other) const {
  if (!parent_)
    return false;
  for (const auto& runner : NodeTraversal::AncestorsOf(*parent_)) {
    if (runner == other)
      return true;
  }
  return false;
}

void Node::PrintTo(std::ostream* ostream) const {
  *ostream << class_name() << '(' << range_ << ", \""
           << EscapedStringPiece16(range_.GetString(), '"') << '"';
  PrintMoreTo(ostream);
  *ostream << ')';
}

void Node::PrintMoreTo(std::ostream* ostream) const {}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  node.PrintTo(&ostream);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (!node)
    return ostream << "(null)";
  return ostream << *node;
}
}  // namespace ast
}  // namespace joana
