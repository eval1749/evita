// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node.h"

#include "base/logging.h"
#include "evita/visuals/dom/ancestors_or_self.h"
#include "evita/visuals/dom/container_node.h"
#include "evita/visuals/dom/document.h"

namespace visuals {

namespace {
int last_sequence_id;
}

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Document* document,
           const base::StringPiece16& node_name,
           const base::StringPiece16& id)
    : document_(document),
      id_(id.as_string()),
      sequence_id_(++last_sequence_id),
      node_name_(node_name.as_string()) {}

Node::Node(Document* document, const base::StringPiece16& node_name)
    : Node(document, node_name, base::StringPiece16()) {}

Node::~Node() {}

bool Node::operator==(const Node& other) const {
  return this == &other;
}

bool Node::operator==(const Node* other) const {
  return this == other;
}

bool Node::operator!=(const Node& other) const {
  return !operator==(other);
}

bool Node::operator!=(const Node* other) const {
  return !operator==(other);
}

bool Node::InDocument() const {
  for (const auto& runner : Node::AncestorsOrSelf(*this)) {
    if (runner->is<Document>())
      return true;
  }
  return false;
}

bool Node::IsDescendantOf(const Node& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  ostream << node.class_name() << ' ' << node.node_name();
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
