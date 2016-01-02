// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_ROOT_BOX_H_
#define EVITA_VISUALS_MODEL_ROOT_BOX_H_

#include <map>

#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/model/box_tree_lifecycle.h"
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

  BoxTreeLifecycle* lifecycle() const { return &lifecycle_; }
  const FloatSize& viewport_size() const { return viewport_size_; }

  Box* GetBoxById(const base::StringPiece16& id) const;
  bool InLayout() const;
  bool InPaint() const;
  bool IsLayoutClean() const;
  bool IsPaintClean() const;

 private:
  void RegisterBoxIdIfNeeded(const Box& box);
  void UnregisterBoxIdIfNeeded(const Box& box);

  std::map<base::string16, Box*> id_map_;
  mutable BoxTreeLifecycle lifecycle_;
  FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(RootBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_ROOT_BOX_H_
