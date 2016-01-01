// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_
#define EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class Box;
class RootBox;

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder
//
class BoxTreeBuilder final {
 public:
  BoxTreeBuilder();
  ~BoxTreeBuilder();

  std::unique_ptr<RootBox> Finish();

  BoxTreeBuilder& Append(std::unique_ptr<Box> child);

 private:
  std::unique_ptr<RootBox> root_box_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_
