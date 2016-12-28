// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_BINDINGS_H_
#define JOANA_AST_BINDINGS_H_

#include <vector>

#include "joana/ast/node.h"

namespace joana {
namespace ast {

class Expression;
class Name;

//
// BindingElement is a base class of binding pattern.
//
class JOANA_AST_EXPORT BindingElement : public Node {
  DECLARE_ABSTRACT_AST_NODE(BindingElement, Node);

 public:
  ~BindingElement() override;

 protected:
  explicit BindingElement(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(BindingElement);
};

//
// ArrayBindingPattern
//
class JOANA_AST_EXPORT ArrayBindingPattern final
    : public NodeTemplate<BindingElement, const Expression*> {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(ArrayBindingPattern, BindingElement);

 public:
  ~ArrayBindingPattern() final;

  const auto& elements() const { return elements_; }
  const Expression& initializer() const { return *member_at<0>(); }

 private:
  ArrayBindingPattern(const SourceCodeRange& range,
                      const std::vector<const BindingElement*>& elements,
                      const Expression& initializer);

  // |elements_| must be the last member variable.
  const NodeListTemplate<const BindingElement> elements_;

  DISALLOW_COPY_AND_ASSIGN(ArrayBindingPattern);
};

//
// BindingCommaElement
//
class JOANA_AST_EXPORT BindingCommaElement final : public BindingElement {
  DECLARE_CONCRETE_AST_NODE(BindingCommaElement, BindingElement);

 public:
  ~BindingCommaElement() final;

 private:
  explicit BindingCommaElement(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(BindingCommaElement);
};

//
// BindingNameElement
//
class JOANA_AST_EXPORT BindingNameElement final
    : public NodeTemplate<BindingElement, const Name*, const Expression*> {
  DECLARE_CONCRETE_AST_NODE(BindingNameElement, BindingElement);

 public:
  ~BindingNameElement() final;

  const Expression& initializer() const { return *member_at<1>(); }
  const Name& name() const { return *member_at<0>(); }

 private:
  BindingNameElement(const SourceCodeRange& range,
                     const Name& name,
                     const Expression& initializer);

  DISALLOW_COPY_AND_ASSIGN(BindingNameElement);
};

//
// BindingProperty is a base class of binding property.
//
class JOANA_AST_EXPORT BindingProperty
    : public NodeTemplate<BindingElement, const Name*, const BindingElement*> {
  DECLARE_CONCRETE_AST_NODE(BindingProperty, BindingElement);

 public:
  ~BindingProperty() override;

  const BindingElement& element() const { return *member_at<1>(); }
  const Name& name() const { return *member_at<0>(); }

 private:
  BindingProperty(const SourceCodeRange& range,
                  const Name& name,
                  const BindingElement& element);

  DISALLOW_COPY_AND_ASSIGN(BindingProperty);
};

//
// BindingRestElement
//
class JOANA_AST_EXPORT BindingRestElement final
    : public NodeTemplate<BindingElement, const BindingElement*> {
  DECLARE_CONCRETE_AST_NODE(BindingRestElement, BindingElement);

 public:
  ~BindingRestElement() final;

  const BindingElement& element() const { return *member_at<0>(); }

 private:
  BindingRestElement(const SourceCodeRange& range,
                     const BindingElement& element);

  DISALLOW_COPY_AND_ASSIGN(BindingRestElement);
};

//
// ObjectBindingPattern
//
// Proposal[1] allows object binding patter to have |BindingRestElement|.
//
// [1] https://github.com/sebmarkbage/ecmascript-rest-spread
//
class JOANA_AST_EXPORT ObjectBindingPattern final
    : public NodeTemplate<BindingElement, const Expression*> {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(ObjectBindingPattern, BindingElement);

 public:
  ~ObjectBindingPattern() final;

  const Expression& initializer() const { return *member_at<0>(); }
  const auto& elements() const { return elements_; }

 private:
  ObjectBindingPattern(const SourceCodeRange& range,
                       const std::vector<const BindingElement*>& elements,
                       const Expression& initializer);

  // |elements_| must the last member variable.
  const NodeListTemplate<const BindingElement> elements_;

  DISALLOW_COPY_AND_ASSIGN(ObjectBindingPattern);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_BINDINGS_H_
