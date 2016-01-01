// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/root_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder
//
BoxTreeBuilder::BoxTreeBuilder() : root_box_(new RootBox()) {}
BoxTreeBuilder::~BoxTreeBuilder() {}

BoxTreeBuilder& BoxTreeBuilder::Append(std::unique_ptr<Box> child) {
  BoxEditor().AppendChild(root_box_.get(), std::move(child));
  return *this;
}

std::unique_ptr<RootBox> BoxTreeBuilder::Finish() {
  DCHECK(root_box_);
  return std::move(root_box_);
}

}  // namespace visuals
