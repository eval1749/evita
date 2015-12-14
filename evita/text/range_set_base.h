// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_SET_BASE_H_
#define EVITA_TEXT_RANGE_SET_BASE_H_

namespace text {

class Offset;
class RangeBase;

class RangeSetBase {
 protected:
  RangeSetBase();
  ~RangeSetBase();

  void set_range(RangeBase* range, Offset start, Offset end);
  void set_range_end(RangeBase* range, Offset end);
  void set_range_start(RangeBase* range, Offset start);
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_SET_BASE_H_
