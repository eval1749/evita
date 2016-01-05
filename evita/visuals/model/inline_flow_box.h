// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_INLINE_FLOW_BOX_H_
#define EVITA_VISUALS_MODEL_INLINE_FLOW_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// InlineFlowBox represents a CSS layout box with "display: inline flow".
//
class InlineFlowBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(InlineFlowBox, ContainerBox);

 public:
  InlineFlowBox(RootBox* root_box, const Node* node);
  explicit InlineFlowBox(RootBox* root_box);
  ~InlineFlowBox() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineFlowBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_INLINE_FLOW_BOX_H_
