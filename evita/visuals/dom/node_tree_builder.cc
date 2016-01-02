// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/text_node.h"

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
    : new_document_(new Document()), document_(new_document_.get()) {
  nodes_.push(document_);
}

NodeTreeBuilder::~NodeTreeBuilder() {
  DCHECK(nodes_.empty());
}

NodeTreeBuilder& NodeTreeBuilder::AddText(const base::StringPiece16& text) {
  return AddInternal(std::make_unique<TextNode>(document_, text));
}

NodeTreeBuilder& NodeTreeBuilder::AddInternal(std::unique_ptr<Node> child) {
  const auto container = nodes_.top()->as<ContainerNode>();
  NodeEditor().AppendChild(container, std::move(child));
  return *this;
}

NodeTreeBuilder& NodeTreeBuilder::Begin(const base::StringPiece16& tag_name) {
  return BeginInternal(std::make_unique<Element>(document_, tag_name));
}

NodeTreeBuilder& NodeTreeBuilder::Begin(const base::StringPiece16& tag_name,
                                        const base::StringPiece16& id) {
  return BeginInternal(std::make_unique<Element>(document_, tag_name, id));
}

NodeTreeBuilder& NodeTreeBuilder::BeginInternal(
    std::unique_ptr<Element> child) {
  const auto new_top = child.get();
  AddInternal(std::move(std::unique_ptr<Node>(child.release())));
  nodes_.push(new_top);
  return *this;
}

std::unique_ptr<Document> NodeTreeBuilder::Build() {
  DCHECK_EQ(1, nodes_.size());
  nodes_.pop();
  return std::move(new_document_);
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

NodeTreeBuilder& NodeTreeBuilder::SetBaseline(float baseline) {
  const auto& text = nodes_.top()->as<TextNode>();
  DCHECK(text) << nodes_.top()->class_name();
  NodeEditor().SetBaseline(text, baseline);
  return *this;
}

NodeTreeBuilder& NodeTreeBuilder::SetStyle(const css::Style& style) {
  NodeEditor().SetStyle(nodes_.top(), style);
  return *this;
}

}  // namespace visuals
