// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_INLINE_BLOCK_BOX_H_
#define EVITA_VISUALS_MODEL_INLINE_BLOCK_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// InlineBlockBox
//
class InlineBlockBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(InlineBlockBox, ContainerBox);

 public:
  InlineBlockBox();
  ~InlineBlockBox() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineBlockBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_INLINE_BLOCK_BOX_H_
