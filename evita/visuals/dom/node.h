// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_H_
#define EVITA_VISUALS_DOM_NODE_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "common/castable.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/gc/collectable.h"
#include "evita/gc/visitor.h"
#include "evita/visuals/dom/nodes_forward.h"

namespace visuals {

class NodeEditor;
class Document;

#define DECLARE_VISUAL_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)          \
  DECLARE_GC_VISITABLE_OBJECT(Node)            \
  friend class NodeEditor;

#define DECLARE_VISUAL_NODE_ABSTRACT_CLASS(self, super) \
  DECLARE_VISUAL_NODE_CLASS(self, super)

#define DECLARE_VISUAL_NODE_FINAL_CLASS(self, super) \
  DECLARE_VISUAL_NODE_CLASS(self, super)             \
 private:                                            \
  void Accept(NodeVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public common::Castable<Node>, public gc::Collectable<Node> {
  DECLARE_VISUAL_NODE_ABSTRACT_CLASS(Node, Castable);

 public:
  class Ancestors;
  class AncestorsOrSelf;
  class Children;
  class Descendants;
  class DescendantsOrSelf;

  virtual ~Node();

  bool operator==(const Node& other) const;
  bool operator==(const Node* other) const;
  bool operator!=(const Node& other) const;
  bool operator!=(const Node* other) const;

  // Tree structure
  Node* next_sibling() const { return next_sibling_; }
  Node* previous_sibling() const { return previous_sibling_; }
  Document* document() const { return document_; }

  // Node identifiers
  const AtomicString& id() const { return id_; }
  int sequence_id() const { return sequence_id_; }
  const AtomicString& node_name() const { return node_name_; }

  // Node tree related values
  ContainerNode* parent() const { return parent_; }

  virtual void Accept(NodeVisitor* visitor) = 0;
  bool InDocument() const;
  bool IsDescendantOf(const Node& other) const;

 protected:
  Node(Document* document,
       const AtomicString& node_name,
       const AtomicString& id);
  Node(Document* document,
       const base::StringPiece16& node_name,
       const base::StringPiece16& id);
  Node(Document* document, const AtomicString& node_name);
  Node(Document* document, const base::StringPiece16& node_name);

 private:
  // gc::Visitable
  void Accept(gc::Visitor* visitor) override;

  Document* document_;
  // User specified string identifier of this node. Multiple nodes can have
  // same string id.
  const AtomicString id_;
  Node* next_sibling_ = nullptr;
  const AtomicString node_name_;
  ContainerNode* parent_ = nullptr;
  Node* previous_sibling_ = nullptr;
  const int sequence_id_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

std::ostream& operator<<(std::ostream& ostream, const Node& node);
std::ostream& operator<<(std::ostream& ostream, const Node* node);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_H_
