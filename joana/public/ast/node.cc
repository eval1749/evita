// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/node.h"

#include "base/strings/utf_string_conversions.h"

namespace joana {
namespace ast {

Node::Node(const SourceCodeRange& location) : range_(location) {}
Node::~Node() = default;

Node* Node::first_child() const {
  return nullptr;
}
Node* Node::last_child() const {
  return nullptr;
}

void Node::PrintTo(std::ostream* ostream) const {
  *ostream << base::UTF16ToUTF8(range_.GetString());
}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  node.PrintTo(&ostream);
  return ostream;
}

}  // namespace ast
}  // namespace joana
