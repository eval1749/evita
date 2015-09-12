// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_INTERVAL_SET_H_
#define EVITA_TEXT_INTERVAL_SET_H_

#include <memory>

#include "base/basictypes.h"
#include "evita/precomp.h"

namespace css {
class Style;
}

namespace text {

class Interval;

class IntervalSet final {
 public:
  explicit IntervalSet(Buffer* buffer);
  ~IntervalSet();

  Interval* GetIntervalAt(Posn offset) const;
  void SetStyle(Posn, Posn, const css::Style& style_values);

 private:
  class Impl;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(IntervalSet);
};

}  // namespace text

#endif  // EVITA_TEXT_INTERVAL_SET_H_
