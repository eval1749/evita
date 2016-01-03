// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder
//
BoxTreeBuilder::BoxTreeBuilder(ContainerBox* container)
    : root_box_(container->root_box()) {
  boxes_.push(container);
}

BoxTreeBuilder::BoxTreeBuilder()
    : new_root_box_(new RootBox(*new Document())),
      root_box_(new_root_box_.get()) {
  boxes_.push(root_box_);
}

BoxTreeBuilder::~BoxTreeBuilder() {
  DCHECK(boxes_.empty());
}

BoxTreeBuilder& BoxTreeBuilder::AddInternal(std::unique_ptr<Box> child) {
  const auto container = boxes_.top()->as<ContainerBox>();
  BoxEditor().AppendChild(container, std::move(child));
  return *this;
}

BoxTreeBuilder& BoxTreeBuilder::BeginInternal(std::unique_ptr<Box> child) {
  const auto new_top = child.get();
  AddInternal(std::move(child));
  boxes_.push(new_top);
  return *this;
}

std::unique_ptr<RootBox> BoxTreeBuilder::Build() {
  DCHECK_EQ(1, boxes_.size());
  boxes_.pop();
  return std::move(new_root_box_);
}

BoxTreeBuilder& BoxTreeBuilder::EndInternal() {
  boxes_.pop();
  return *this;
}

void BoxTreeBuilder::FinishInternal(Box* box) {
  DCHECK_EQ(1, boxes_.size());
  DCHECK_EQ(boxes_.top(), box);
  boxes_.pop();
}

BoxTreeBuilder& BoxTreeBuilder::SetBaseline(float baseline) {
  const auto& text = boxes_.top()->as<TextBox>();
  DCHECK(text) << boxes_.top()->class_name();
  BoxEditor().SetBaseline(text, baseline);
  return *this;
}

BoxTreeBuilder& BoxTreeBuilder::SetStyle(const css::Style& style) {
  BoxEditor().SetStyle(boxes_.top(), style);
  return *this;
}

}  // namespace visuals
