// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/shape.h"
#include "evita/visuals/dom/text.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NodeTreeBuilder
//
NodeTreeBuilder::NodeTreeBuilder(ContainerNode* container)
    : document_(container->document()) {
  nodes_.push(container);
}

NodeTreeBuilder::NodeTreeBuilder()
    : new_document_(new Document()), document_(new_document_) {
  nodes_.push(document_);
}

NodeTreeBuilder::~NodeTreeBuilder() {
  DCHECK(nodes_.empty());
}

NodeTreeBuilder& NodeTreeBuilder::AddText(const base::StringPiece16& text) {
  return AddInternal(new Text(document_, text));
}

NodeTreeBuilder& NodeTreeBuilder::AddShape(const std::vector<uint8_t>& value) {
  return AddInternal(new Shape(document_, ShapeData(value)));
}

NodeTreeBuilder& NodeTreeBuilder::AddInternal(Node* child) {
  const auto container = nodes_.top()->as<ContainerNode>();
  NodeEditor().AppendChild(container, child);
  return *this;
}

NodeTreeBuilder& NodeTreeBuilder::Begin(const base::StringPiece16& tag_name) {
  return BeginInternal(new Element(document_, tag_name));
}

NodeTreeBuilder& NodeTreeBuilder::Begin(const base::StringPiece16& tag_name,
                                        const base::StringPiece16& id) {
  return BeginInternal(new Element(document_, tag_name, id));
}

NodeTreeBuilder& NodeTreeBuilder::BeginInternal(ElementNode* child) {
  AddInternal(child);
  nodes_.push(child);
  return *this;
}

Document* NodeTreeBuilder::Build() {
  DCHECK_EQ(1, nodes_.size()) << " You should call End() for " << nodes_.top();
  nodes_.pop();
  const auto document = new_document_;
  new_document_ = nullptr;
  return document;
}

NodeTreeBuilder& NodeTreeBuilder::ClassList(
    const std::vector<base::string16>& names) {
  auto const element = nodes_.top()->as<Element>();
  for (const auto& name : names)
    NodeEditor().AddClass(element, name);
  return *this;
}

NodeTreeBuilder& NodeTreeBuilder::End(const base::StringPiece16& tag_name) {
  auto const element = nodes_.top()->as<Element>();
  DCHECK_EQ(tag_name.as_string(), element->tag_name());
  nodes_.pop();
  return *this;
}

void NodeTreeBuilder::Finish(ContainerNode* node) {
  DCHECK_EQ(1, nodes_.size());
  DCHECK_EQ(nodes_.top(), node);
  nodes_.pop();
}

NodeTreeBuilder& NodeTreeBuilder::SetInlineStyle(const css::Style& style) {
  NodeEditor().SetInlineStyle(nodes_.top()->as<Element>(), style);
  return *this;
}

}  // namespace visuals
