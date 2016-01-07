// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_FLOW_BOX_H_
#define EVITA_VISUALS_LAYOUT_FLOW_BOX_H_

#include "evita/visuals/layout/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FlowBox  represents a CSS layout box with "display: block flow".
//
class FlowBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(FlowBox, ContainerBox);

 public:
  FlowBox(RootBox* root_box, const Node* node);
  explicit FlowBox(RootBox* root_box);
  ~FlowBox() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(FlowBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_FLOW_BOX_H_
