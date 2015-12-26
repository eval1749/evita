// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
#define EVITA_VISUALS_MODEL_CONTAINER_BOX_H_

#include <vector>

#include "evita/visuals/model/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
class ContainerBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(ContainerBox, Box);

 public:
  ~ContainerBox() override;

  const std::vector<Box*> child_boxes() const { return child_boxes_; }

  void AppendChild(Box* child);
  void RemoveChild(Box* child);

 protected:
  ContainerBox();

 private:
  std::vector<Box*> child_boxes_;

  DISALLOW_COPY_AND_ASSIGN(ContainerBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
