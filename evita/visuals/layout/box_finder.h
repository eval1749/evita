// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_FINDER_H_
#define EVITA_VISUALS_LAYOUT_BOX_FINDER_H_

#include <iosfwd>

#include "base/macros.h"
#include "evita/visuals/geometry/float_point.h"

namespace visuals {

class Box;
class RootBox;

//////////////////////////////////////////////////////////////////////
//
// BoxFinder
//
class BoxFinder final {
 public:
  struct Result {
    Box* box = nullptr;
    // |point| in box coordinate space.
    FloatPoint point;

    Result(Box* box_in, const FloatPoint& point_in)
        : box(box_in), point(point_in) {}
    Result() {}

    bool operator==(const Result& other) const {
      return box == other.box && point == other.point;
    }

    bool operator!=(const Result& other) const { return !operator==(other); }
  };

  explicit BoxFinder(const RootBox& root_box);
  ~BoxFinder();

  Result FindByPoint(const FloatPoint& point) const;

 private:
  const RootBox& root_box_;

  DISALLOW_COPY_AND_ASSIGN(BoxFinder);
};

std::ostream& operator<<(std::ostream& ostream,
                         const BoxFinder::Result& result);

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_FINDER_H_
