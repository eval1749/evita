// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BLOCK_FLOW_BOX_H_
#define EVITA_VISUALS_MODEL_BLOCK_FLOW_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BlockFlowBox
//
class BlockFlowBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(BlockFlowBox, ContainerBox);

 public:
  BlockFlowBox(RootBox* root_box, const base::StringPiece16& id);
  explicit BlockFlowBox(RootBox* root_box);
  ~BlockFlowBox() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(BlockFlowBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BLOCK_FLOW_BOX_H_
