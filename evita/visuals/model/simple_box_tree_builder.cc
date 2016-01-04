// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/simple_box_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// SimpleBoxTreeBuilder
//
SimpleBoxTreeBuilder::SimpleBoxTreeBuilder(ContainerBox* container)
    : root_box_(container->root_box()) {
  boxes_.push(container);
}

SimpleBoxTreeBuilder::SimpleBoxTreeBuilder()
    : new_root_box_(new RootBox(*new Document())),
      root_box_(new_root_box_.get()) {
  boxes_.push(root_box_);
}

SimpleBoxTreeBuilder::~SimpleBoxTreeBuilder() {
  DCHECK(boxes_.empty());
}

SimpleBoxTreeBuilder& SimpleBoxTreeBuilder::AddInternal(
    std::unique_ptr<Box> child) {
  const auto container = boxes_.top()->as<ContainerBox>();
  BoxEditor().AppendChild(container, std::move(child));
  return *this;
}

SimpleBoxTreeBuilder& SimpleBoxTreeBuilder::BeginInternal(
    std::unique_ptr<Box> child) {
  const auto new_top = child.get();
  AddInternal(std::move(child));
  boxes_.push(new_top);
  return *this;
}

std::unique_ptr<RootBox> SimpleBoxTreeBuilder::Build() {
  DCHECK_EQ(1, boxes_.size());
  boxes_.pop();
  return std::move(new_root_box_);
}

SimpleBoxTreeBuilder& SimpleBoxTreeBuilder::EndInternal() {
  boxes_.pop();
  return *this;
}

void SimpleBoxTreeBuilder::FinishInternal(Box* box) {
  DCHECK_EQ(1, boxes_.size());
  DCHECK_EQ(boxes_.top(), box);
  boxes_.pop();
}

SimpleBoxTreeBuilder& SimpleBoxTreeBuilder::SetBaseline(float baseline) {
  const auto& text = boxes_.top()->as<TextBox>();
  DCHECK(text) << boxes_.top()->class_name();
  BoxEditor().SetBaseline(text, baseline);
  return *this;
}

SimpleBoxTreeBuilder& SimpleBoxTreeBuilder::SetStyle(const css::Style& style) {
  BoxEditor().SetStyle(boxes_.top(), style);
  return *this;
}

}  // namespace visuals
