// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_LINE_BOX_H_
#define EVITA_VISUALS_MODEL_LINE_BOX_H_

#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// LineBox
//
class LineBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(LineBox, ContainerBox);

 public:
  LineBox(RootBox* root_box, const base::StringPiece16& id);
  explicit LineBox(RootBox* root_box);
  ~LineBox() final;

 private:
  // Box
  FloatSize ComputePreferredSize() const final;

  DISALLOW_COPY_AND_ASSIGN(LineBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_LINE_BOX_H_
