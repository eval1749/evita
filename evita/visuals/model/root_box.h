// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_ROOT_BOX_H_
#define EVITA_VISUALS_MODEL_ROOT_BOX_H_

#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/model/box_tree_lifecycle.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

class Document;

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
class RootBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(RootBox, ContainerBox);

 public:
  explicit RootBox(const Document& document);
  ~RootBox() final;

  BoxTreeLifecycle* lifecycle() const { return &lifecycle_; }
  const FloatSize& viewport_size() const { return viewport_size_; }

  bool InLayout() const;
  bool InPaint() const;
  bool IsLayoutClean() const;
  bool IsPaintClean() const;

 private:
  mutable BoxTreeLifecycle lifecycle_;
  FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(RootBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_ROOT_BOX_H_
