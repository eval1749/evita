// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_ROOT_BOX_H_
#define EVITA_VISUALS_LAYOUT_ROOT_BOX_H_

#include <memory>

#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/layout/container_box.h"

namespace visuals {

class BoxSelection;
class Document;
class ViewLifecycle;

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
class RootBox final : public ContainerBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(RootBox, ContainerBox);

 public:
  explicit RootBox(ViewLifecycle* lifecycle);
  ~RootBox() final;

  const Document& document() const;
  const BoxSelection& selection() const { return *selection_; }
  ViewLifecycle* lifecycle() const { return lifecycle_; }
  const FloatSize& viewport_size() const { return viewport_size_; }

  bool InLayout() const;
  bool InPaint() const;
  bool InTreeRebuild() const;
  bool IsLayoutClean() const;
  bool IsPaintClean() const;

 private:
  bool is_selection_changed_ = false;
  ViewLifecycle* const lifecycle_;
  FloatSize viewport_size_;
  std::unique_ptr<BoxSelection> selection_;

  DISALLOW_COPY_AND_ASSIGN(RootBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_ROOT_BOX_H_
