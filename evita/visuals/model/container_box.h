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
  const FloatRect& content_bounds() const { return content_bounds_; }

  void AppendChild(Box* child);
  void RemoveChild(Box* child);

 protected:
  ContainerBox(Display display,
               const FloatRect& bounds,
               const FloatRect& content_bounds);

 private:
  std::vector<Box*> child_boxes_;
  FloatRect content_bounds_;

  DISALLOW_COPY_AND_ASSIGN(ContainerBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
