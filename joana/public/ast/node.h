// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_H_
#define JOANA_PUBLIC_AST_NODE_H_

#include <array>
#include <iosfwd>
#include <tuple>
#include <type_traits>

#include "base/macros.h"
#include "joana/public/castable.h"
#include "joana/public/memory/zone_allocated.h"
#include "joana/public/memory/zone_vector.h"
#include "joana/public/public_export.h"
#include "joana/public/source_code_range.h"
#include "joana/public/visitable.h"

namespace joana {
namespace ast {

class ContainerNode;
enum class NameId;
class NodeVisitor;
enum class PunctuatorKind;

class JOANA_PUBLIC_EXPORT Node : public Castable<Node>,
                                 public Visitable<NodeVisitor>,
                                 public ZoneAllocated {
  DECLARE_CASTABLE_CLASS(Node, Castable);

 public:
  virtual ~Node();

  bool operator==(const Node& other) const;
  bool operator==(const Node* other) const;
  bool operator!=(const Node& other) const;
  bool operator!=(const Node* other) const;
  bool operator==(NameId name_id) const;
  bool operator!=(NameId name_id) const;
  bool operator==(PunctuatorKind kind) const;
  bool operator!=(PunctuatorKind kind) const;

  virtual Node* first_child() const;
  virtual Node* last_child() const;
  Node* next_sibling() const { return next_sibling_; }
  ContainerNode* parent() const { return parent_; }
  Node* previous_sibling() const { return previous_sibling_; }

  const SourceCodeRange& range() const { return range_; }

  // TODO(eval1749): We should make |ChildAt()| as pure virtual function.
  virtual const Node& ChildAt(size_t index) const;
  bool Contains(const Node& other) const;
  // TODO(eval1749): We should make |CountChildNodes()| as pure virtual
  // function.
  virtual size_t CountChildNodes() const;
  bool IsDescendantOf(const Node& other) const;

  void PrintTo(std::ostream* ostream) const;

 protected:
  explicit Node(const SourceCodeRange& range);

  virtual void PrintMoreTo(std::ostream* ostream) const;

 private:
  friend class NodeEditor;

  // TODO(eval1749): We should not use tree. We should use fixed node and
  // variable node.
  Node* next_sibling_ = nullptr;
  ContainerNode* parent_ = nullptr;
  Node* previous_sibling_ = nullptr;
  const SourceCodeRange range_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

JOANA_PUBLIC_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                             const Node& node);
JOANA_PUBLIC_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                             const Node* node);

#define DECLARE_AST_NODE(name, base)  \
  DECLARE_CASTABLE_CLASS(name, base); \
  friend class NodeEditor;            \
  friend class NodeFactory;

#define DECLARE_ABSTRACT_AST_NODE(name, base) DECLARE_AST_NODE(name, base);

#define DECLARE_CONCRETE_AST_NODE(name, base) \
  DECLARE_AST_NODE(name, base);               \
  void Accept(NodeVisitor* visitor) final;

//
// NodeTemplate represents fixed number of child nodes.
//
template <typename Base, typename... Members>
class NodeTemplate : public Base {
 protected:
  template <typename... Params>
  explicit NodeTemplate(const std::tuple<Members...>& members, Params... params)
      : Base(params...), members_(members) {}

  ~NodeTemplate() override = default;

 protected:
  template <size_t kIndex>
  auto member_at() const {
    return std::get<kIndex>(members_);
  }

 private:
  std::tuple<Members...> members_;

  DISALLOW_COPY_AND_ASSIGN(NodeTemplate);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_H_
