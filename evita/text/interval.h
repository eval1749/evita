// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_INTERVAL_H_
#define EVITA_TEXT_INTERVAL_H_

#include <ostream>

#include "base/macros.h"
#include "evita/css/style.h"
#include "evita/text/range_base.h"

namespace text {

class Offset;

//////////////////////////////////////////////////////////////////////
//
// Interval
//
class Interval final : public RangeBase {
 public:
  Interval(const Interval& other);
  Interval(Offset start, Offset end);
  ~Interval();

  const css::Style& style() const { return style_; }
  void set_style(const css::Style& style);

 private:
  css::Style style_;

  DISALLOW_ASSIGN(Interval);
};

}  // namespace text

namespace std {
ostream& operator<<(ostream& ostream, const text::Interval& interval);
ostream& operator<<(ostream& ostream, const text::Interval* interval);
}  // namespace std

#endif  // EVITA_TEXT_INTERVAL_H_
