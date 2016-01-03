// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node.h"

#include "base/logging.h"
#include "evita/visuals/dom/container_node.h"
#include "evita/visuals/dom/document.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Document* document,
           const base::StringPiece16& tag_name,
           const base::StringPiece16& id)
    : id_(id.as_string()),
      tag_name_(tag_name.as_string()),
      document_(document) {}

Node::Node(Document* document, const base::StringPiece16& tag_name)
    : Node(document, tag_name, base::StringPiece16()) {}

Node::~Node() {}

bool Node::IsDescendantOf(const Node& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  ostream << node.class_name() << ' ' << node.tag_name();
  if (!node.id().empty())
    ostream << " id=\"" << node.id() << '"';
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (!node)
    return ostream << "nullptr";
  return ostream << *node;
}

// gc::Visitable
void Node::Accept(gc::Visitor* visitor) {
  visitor->Visit(next_sibling_);
  visitor->Visit(previous_sibling_);
  visitor->Visit(parent_);
  visitor->Visit(document_);
}

}  // namespace visuals
