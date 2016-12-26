// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/jsdoc_nodes.h"

#include "joana/ast/tokens.h"

namespace joana {
namespace ast {

//
// JsDocDocument
//
JsDocDocument::JsDocDocument(const SourceCodeRange& range,
                             const std::vector<const JsDocNode*>& elements)
    : JsDocNode(range), elements_(elements) {}

JsDocDocument::~JsDocDocument() = default;

//
// JsDocName
//
JsDocName::JsDocName(const Name& name) : JsDocNode(name.range()), name_(name) {}
JsDocName::~JsDocName() = default;

//
// JsDocNode
//
JsDocNode::JsDocNode(const SourceCodeRange& range) : Node(range) {}

JsDocNode::~JsDocNode() = default;

//
// JsDocTag
//
JsDocTag::JsDocTag(const SourceCodeRange& range,
                   const Name& name,
                   const std::vector<const JsDocNode*>& parameters)
    : JsDocNode(range), name_(name), parameters_(parameters) {}

JsDocTag::~JsDocTag() = default;

//
// JsDocText
//
JsDocText::JsDocText(const SourceCodeRange& range) : JsDocNode(range) {}
JsDocText::~JsDocText() = default;

//
// JsDocType
//
JsDocType::JsDocType(const SourceCodeRange& range, const Type& type)
    : JsDocNode(range), type_(type) {}

JsDocType::~JsDocType() = default;

}  // namespace ast
}  // namespace joana
