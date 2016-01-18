// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_HIT_TEST_RESULT_H_
#define EVITA_VISUALS_VIEW_PUBLIC_HIT_TEST_RESULT_H_

#include <iosfwd>

#include "evita/visuals/geometry/float_point.h"

namespace visuals {

class Box;
class Node;

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
class HitTestResult final {
 public:
  HitTestResult(Box* box, const FloatPoint& point);
  HitTestResult(const HitTestResult& other);
  HitTestResult();
  ~HitTestResult();

  explicit operator bool() const { return box_ != nullptr; }

  bool operator==(const HitTestResult& other) const;
  bool operator!=(const HitTestResult& other) const;

  Box* box() const { return box_; }
  Node* node() const;
  const FloatPoint& point() const { return point_; }

 private:
  Box* box_ = nullptr;
  // |point| in box coordinate space.
  FloatPoint point_;
};

std::ostream& operator<<(std::ostream& ostream, const HitTestResult& result);

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_HIT_TEST_RESULT_H_
