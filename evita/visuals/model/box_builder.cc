// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_builder.h"

#include "base/logging.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxBuilder
//
BoxBuilder::BoxBuilder(std::unique_ptr<Box> box) : box_(std::move(box)) {}

BoxBuilder::~BoxBuilder() {
  DCHECK(!box_) << "You should call Build()";
}

std::unique_ptr<Box> BoxBuilder::Finish() {
  return std::move(box_);
}

BoxBuilder& BoxBuilder::Append(std::unique_ptr<Box> child) {
  const auto& container = box_->as<ContainerBox>();
  DCHECK(container) << *box_ << " should be a container.";
  BoxEditor().AppendChild(container, std::move(child));
  return *this;
}

BoxBuilder& BoxBuilder::SetBaseline(float baseline) {
  const auto& text = box_->as<TextBox>();
  DCHECK(text) << box_->class_name();
  BoxEditor().SetBaseline(text, baseline);
  return *this;
}

BoxBuilder& BoxBuilder::SetColor(const FloatColor& color) {
  const auto& text = box_->as<TextBox>();
  DCHECK(text) << box_->class_name();
  BoxEditor().SetColor(text, color);
  return *this;
}

}  // namespace visuals
