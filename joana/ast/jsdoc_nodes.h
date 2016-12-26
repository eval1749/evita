// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_JSDOC_NODES_H_
#define JOANA_AST_JSDOC_NODES_H_

#include <vector>

#include "joana/ast/node.h"
#include "joana/ast/node_forward.h"

namespace joana {
namespace ast {

//
// JsDocNode
//
class JOANA_AST_EXPORT JsDocNode : public Node {
  DECLARE_ABSTRACT_AST_NODE(JsDocNode, Node);

 public:
  ~JsDocNode() override;

 protected:
  explicit JsDocNode(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(JsDocNode);
};

//
// JsDocDocument
//
class JOANA_AST_EXPORT JsDocDocument : public JsDocNode {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(JsDocDocument, JsDocNode);

 public:
  ~JsDocDocument() override;

  const NodeListTemplate<const JsDocNode>& elements() const {
    return elements_;
  }

 private:
  explicit JsDocDocument(const SourceCodeRange& range,
                         const std::vector<const JsDocNode*>& elements);

  NodeListTemplate<const JsDocNode> elements_;

  DISALLOW_COPY_AND_ASSIGN(JsDocDocument);
};

//
// JsDocName
//
class JOANA_AST_EXPORT JsDocName : public JsDocNode {
  DECLARE_CONCRETE_AST_NODE(JsDocName, JsDocNode);

 public:
  ~JsDocName() override;

  const Name& name() const { return name_; }

 private:
  explicit JsDocName(const Name& name);

  const Name& name_;

  DISALLOW_COPY_AND_ASSIGN(JsDocName);
};

//
// JsDocTag
//
class JOANA_AST_EXPORT JsDocTag : public JsDocNode {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(JsDocTag, JsDocNode);

 public:
  ~JsDocTag() override;

  const Name& name() const { return name_; }
  const NodeListTemplate<const JsDocNode>& parameters() const {
    return parameters_;
  }

 private:
  JsDocTag(const SourceCodeRange& range,
           const Name& name,
           const std::vector<const JsDocNode*>& parameters);

  const Name& name_;
  NodeListTemplate<const JsDocNode> parameters_;

  DISALLOW_COPY_AND_ASSIGN(JsDocTag);
};

//
// JsDocText
//
class JOANA_AST_EXPORT JsDocText : public JsDocNode {
  DECLARE_CONCRETE_AST_NODE(JsDocText, JsDocNode);

 public:
  ~JsDocText() override;

 private:
  explicit JsDocText(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(JsDocText);
};

//
// JsDocType
//
class JOANA_AST_EXPORT JsDocType : public JsDocNode {
  DECLARE_CONCRETE_AST_NODE(JsDocType, JsDocNode);

 public:
  ~JsDocType() override;

  const Type& type() const { return type_; }

 private:
  JsDocType(const SourceCodeRange& range, const Type& type);

  const Type& type_;

  DISALLOW_COPY_AND_ASSIGN(JsDocType);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_JSDOC_NODES_H_
