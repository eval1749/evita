// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_finder.h"

#include "base/logging.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"

namespace visuals {

namespace {

HitTestResult WalkInTree(const FloatPoint& point, const Box* box) {
  const auto& point_in_box =
      point - FloatSize(box->bounds().x(), box->bounds().y());
  if (!box->bounds().Contains(point))
    return HitTestResult();
  const auto container = box->as<ContainerBox>();
  if (container) {
    for (const auto child : container->child_boxes()) {
      const auto& result = WalkInTree(point_in_box, child);
      if (result.box()) {
        if (result.node())
          return result;
        break;
      }
    }
  }
  return HitTestResult(const_cast<Box*>(box), point_in_box);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BoxFinder
//
BoxFinder::BoxFinder(const RootBox& root_box) : root_box_(root_box) {}
BoxFinder::~BoxFinder() {}

HitTestResult BoxFinder::FindByPoint(const FloatPoint& point) const {
  DCHECK(root_box_.IsLayoutClean());
  return WalkInTree(point, &root_box_);
}

}  // namespace visuals
