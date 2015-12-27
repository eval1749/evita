// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
#define EVITA_VISUALS_MODEL_CONTAINER_BOX_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "evita/visuals/model/box.h"

namespace visuals {

class BoxEditor;

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
class ContainerBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(ContainerBox, Box);

 public:
  ~ContainerBox() override;

  const std::vector<Box*> child_boxes() const { return child_boxes_; }

 protected:
  ContainerBox();

 private:
  friend class BoxEditor;

  // For ease of using list of child boxes, we don't use |std::unique_ptr<Box>|
  // for elements of |std::vector<T>|.
  std::vector<Box*> child_boxes_;

  DISALLOW_COPY_AND_ASSIGN(ContainerBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
