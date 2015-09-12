// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_BASE_H_
#define EVITA_TEXT_RANGE_BASE_H_

#include "evita/precomp.h"

namespace text {

class RangeSetBase;

class RangeBase {
 public:
  Posn end() const { return end_; }
  Posn start() const { return start_; }

  bool Contains(Posn offset) const;

 protected:
  RangeBase(Posn start, Posn end);
  RangeBase(const RangeBase& other);
  ~RangeBase();

 private:
  friend class RangeSetBase;

  Posn end_;
  Posn start_;
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_BASE_H_
