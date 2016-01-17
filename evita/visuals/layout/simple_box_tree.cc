// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/simple_box_tree.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_lifecycle.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// SimpleBoxTree
//
SimpleBoxTree::SimpleBoxTree(const Document& document)
    : document_(document),
      lifecycle_(new DocumentLifecycle(document_)),
      root_box_(new RootBox(lifecycle_.get())) {
  boxes_.push(root_box_.get());
}

SimpleBoxTree::SimpleBoxTree() : SimpleBoxTree(*new Document()) {
  own_document_ = const_cast<Document*>(&document_);
}

SimpleBoxTree::~SimpleBoxTree() {
  BoxEditor().RemoveDescendants(root_box_.get());
  boxes_.pop();
  DCHECK(boxes_.empty());
}

SimpleBoxTree& SimpleBoxTree::AddInternal(std::unique_ptr<Box> child) {
  const auto container = boxes_.top()->as<ContainerBox>();
  BoxEditor().AppendChild(container, child.release());
  return *this;
}

SimpleBoxTree& SimpleBoxTree::BeginInternal(std::unique_ptr<Box> child) {
  const auto new_top = child.get();
  AddInternal(std::move(child));
  boxes_.push(new_top);
  return *this;
}

SimpleBoxTree& SimpleBoxTree::EndInternal() {
  boxes_.pop();
  return *this;
}

void SimpleBoxTree::FinishInternal(Box* box) {
  DCHECK_EQ(1, boxes_.size());
  DCHECK_EQ(boxes_.top(), box);
  boxes_.pop();
}

SimpleBoxTree& SimpleBoxTree::SetBaseline(float baseline) {
  const auto& text = boxes_.top()->as<TextBox>();
  DCHECK(text) << boxes_.top()->class_name();
  BoxEditor().SetBaseline(text, baseline);
  return *this;
}

SimpleBoxTree& SimpleBoxTree::SetStyle(const css::Style& style) {
  BoxEditor().SetStyle(boxes_.top(), style);
  return *this;
}

}  // namespace visuals
