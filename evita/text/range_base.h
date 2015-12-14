// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_BASE_H_
#define EVITA_TEXT_RANGE_BASE_H_

#include "evita/text/offset.h"

namespace text {

class RangeSetBase;

//////////////////////////////////////////////////////////////////////
//
// RangeSetBase
//
class RangeBase {
 public:
  Offset end() const { return end_; }
  Offset start() const { return start_; }

  bool Contains(Offset offset) const;

 protected:
  RangeBase(Offset start, Offset end);
  RangeBase(const RangeBase& other);
  ~RangeBase();

 private:
  friend class RangeSetBase;

  Offset end_;
  Offset start_;
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_BASE_H_
