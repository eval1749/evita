// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_SET_BASE_H_
#define EVITA_TEXT_RANGE_SET_BASE_H_

#include "evita/precomp.h"

namespace text {

class RangeBase;

class RangeSetBase {
 protected:
  RangeSetBase();
  ~RangeSetBase();

  void set_range(RangeBase* range, Posn start, Posn end);
  void set_range_end(RangeBase* range, Posn end);
  void set_range_start(RangeBase* range, Posn start);
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_SET_BASE_H_
