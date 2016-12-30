// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_H_
#define JOANA_AST_NODE_H_

#include <array>
#include <iosfwd>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "base/logging.h"
#include "base/macros.h"
#include "joana/ast/ast_export.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/memory/zone_vector.h"
#include "joana/base/source_code_range.h"
#include "joana/base/visitable.h"

namespace joana {
namespace ast {

class ContainerNode;
enum class NameId;
class NodeVisitor;
enum class PunctuatorKind;

class JOANA_AST_EXPORT Node : public Castable<Node>,
                              public ConstVisitable<NodeVisitor>,
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

  const SourceCode& source_code() const { return range_.source_code(); }
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

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Node& node);
JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Node* node);

#define DECLARE_AST_NODE(name, base)  \
  DECLARE_CASTABLE_CLASS(name, base); \
  friend class NodeEditor;            \
  friend class NodeFactory;

#define DECLARE_ABSTRACT_AST_NODE(name, base) DECLARE_AST_NODE(name, base);

#define DECLARE_CONCRETE_AST_NODE(name, base) \
  DECLARE_AST_NODE(name, base);               \
  void Accept(NodeVisitor* visitor) const final;

#define DECLARE_CONCRETE_AST_NODE_WITH_LIST(name, base)              \
  DECLARE_CONCRETE_AST_NODE(name, base)                              \
 public:                                                             \
  void* operator new(size_t size, void* pointer) { return pointer; } \
  void* operator new(size_t, Zone*) = delete;

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

//
// NodeListTemplate
//
template <typename Element>
class NodeListTemplate {
 public:
  class Iterator final
      : public std::iterator<std::input_iterator_tag, Element> {
   public:
    Iterator(const Iterator& other)
        : index_(other.index_), owner_(other.owner_) {}
    ~Iterator() = default;

    reference operator*() const {
      DCHECK_LT(index_, owner_->size());
      return *owner_->elements_[index_];
    }

    iterator& operator++() {
      DCHECK_LT(index_, owner_->size());
      ++index_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      DCHECK_EQ(owner_, other.owner_);
      return index_ == other.index_;
    }

    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    friend class NodeListTemplate;

    Iterator(const NodeListTemplate* owner, size_t index)
        : index_(index), owner_(owner) {}

    size_t index_;
    const NodeListTemplate* owner_;
  };

  explicit NodeListTemplate(const std::vector<Element*>& elements)
      : size_(elements.size()) {
    ::memcpy(elements_, elements.data(), sizeof(Element*) * size_);
  }

  ~NodeListTemplate() = default;

  Iterator begin() const { return Iterator(this, 0); }
  bool empty() const { return size_ == 0; }
  Iterator end() const { return Iterator(this, size_); }
  size_t size() const { return size_; }

 private:
  const size_t size_;
  Element* elements_[1];

  DISALLOW_COPY_AND_ASSIGN(NodeListTemplate);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_H_
