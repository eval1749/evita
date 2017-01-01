// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/jsdoc_syntaxes.h"

#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"

namespace joana {
namespace ast {

//
// JsDocDocumentSyntax
//
JsDocDocumentSyntax::JsDocDocumentSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::JsDocDocument,
                     Format::Builder().set_is_variadic(true).Build()) {}

JsDocDocumentSyntax::~JsDocDocumentSyntax() = default;

//
// JsDocTagSyntax
//
JsDocTagSyntax::JsDocTagSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::JsDocTag,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

JsDocTagSyntax::~JsDocTagSyntax() = default;

const Node& JsDocTagSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::JsDocTag);
  return node.child_at(0);
}

ChildNodes JsDocTagSyntax::OperandsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::JsDocTag);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

//
// JsDocTextSyntax
//
JsDocTextSyntax::JsDocTextSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::JsDocText,
                     Format::Builder().Build()) {}

JsDocTextSyntax::~JsDocTextSyntax() = default;

}  // namespace ast
}  // namespace joana
