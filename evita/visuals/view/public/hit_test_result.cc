// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/view/public/hit_test_result.h"

#include "evita/visuals/layout/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
HitTestResult::HitTestResult(Box* box, const FloatPoint& point)
    : box_(box), point_(point) {}

HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.box_, other.point_) {}

HitTestResult::HitTestResult() {}
HitTestResult::~HitTestResult() {}

bool HitTestResult::operator==(const HitTestResult& other) const {
  return box_ == other.box_ && point_ == other.point_;
}

bool HitTestResult::operator!=(const HitTestResult& other) const {
  return !operator==(other);
}

Node* HitTestResult::node() const {
  return box_ ? const_cast<Node*>(box_->node()) : nullptr;
}

std::ostream& operator<<(std::ostream& ostream, const HitTestResult& result) {
  return ostream << '{' << result.box() << ", " << result.point() << '}';
}

}  // namespace visuals
