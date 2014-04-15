// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_interval_h)
#define INCLUDE_evita_text_interval_h

#include "evita/css/style.h"
#include "evita/text/range_base.h"

namespace text {

class Interval : public RangeBase {
  private: css::Style style_;

  public: Interval(const Interval& other);
  public: Interval(Posn start, Posn end);
  public: ~Interval();

  public: const css::Style& style() const { return style_; }
  public: void set_style(const css::Style& style);

  DISALLOW_ASSIGN(Interval);
};

}  // namespace text

#include <ostream>

namespace std {
ostream& operator<<(ostream& ostream, const text::Interval& interval);
ostream& operator<<(ostream& ostream, const text::Interval* interval);
}  // namespace std

#endif //!defined(INCLUDE_evita_text_interval_h)
