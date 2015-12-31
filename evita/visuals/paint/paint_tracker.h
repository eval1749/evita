// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_PAINT_PAINT_TRACKER_H_
#define EVITA_VISUALS_PAINT_PAINT_TRACKER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

class Box;

//////////////////////////////////////////////////////////////////////
//
// PaintTracker
//
class PaintTracker final {
 public:
  explicit PaintTracker(const Box& root_box);
  ~PaintTracker();

  // When display is no longer hold painted tree, e.g. viewport bounds is
  // changed.
  void Clear();

  // Returns exposed rectangles by new layout.
  std::vector<FloatRect> Update();

 private:
  class Impl;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(PaintTracker);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_PAINT_PAINT_TRACKER_H_
