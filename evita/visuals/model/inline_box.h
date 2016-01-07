// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_INLINE_BOX_H_
#define EVITA_VISUALS_MODEL_INLINE_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// InlineBox represents a CSS layout box with "display: inline flow".
//
class InlineBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(InlineBox, ContainerBox);

 public:
  InlineBox(RootBox* root_box, const Node* node);
  explicit InlineBox(RootBox* root_box);
  ~InlineBox() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_INLINE_BOX_H_
