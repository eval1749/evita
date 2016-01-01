// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_ROOT_BOX_H_
#define EVITA_VISUALS_MODEL_ROOT_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
class RootBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(RootBox, ContainerBox);

 public:
  RootBox();
  ~RootBox() final;

 private:
  // Box
  FloatSize ComputePreferredSize() const final;

  DISALLOW_COPY_AND_ASSIGN(RootBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_ROOT_BOX_H_
