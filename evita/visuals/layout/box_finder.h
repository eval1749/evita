// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_FINDER_H_
#define EVITA_VISUALS_LAYOUT_BOX_FINDER_H_

#include "base/macros.h"
#include "evita/visuals/view/public/hit_test_result.h"

namespace visuals {

class Box;
class RootBox;

//////////////////////////////////////////////////////////////////////
//
// BoxFinder
//
class BoxFinder final {
 public:
  explicit BoxFinder(const RootBox& root_box);
  ~BoxFinder();

  HitTestResult FindByPoint(const FloatPoint& point) const;

 private:
  const RootBox& root_box_;

  DISALLOW_COPY_AND_ASSIGN(BoxFinder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_FINDER_H_
