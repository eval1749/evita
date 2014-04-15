// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_interval_set_h)
#define INCLUDE_evita_text_interval_set_h

#include <memory>

namespace css {
class Style;
}

namespace text {

class Interval;

class IntervalSet {
  // PImpl Idiom, see "interval_set.cc" for implementation.
  public: class Impl;
  private: std::unique_ptr<Impl> impl_;

  public: IntervalSet(Buffer* buffer);
  public: ~IntervalSet();

  public: Interval* GetIntervalAt(Posn offset) const;
  public: void SetStyle(Posn, Posn, const css::Style& style_values);

  DISALLOW_COPY_AND_ASSIGN(IntervalSet);
};

}   // text

#endif //!defined(INCLUDE_evita_text_buffer_mutation_observer_h)
