// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_interval_h)
#define INCLUDE_evita_text_interval_h

#include "evita/ed_BinTree.h"
#include "evita/css/style.h"
#include "evita/li_util.h"
#include "evita/text/interval_set.h"

namespace text {

class Interval : public BinaryTree<Interval>::NodeBase,
                 public DoubleLinkedNode_<Interval> {
  friend class IntervalSet::Impl;

  private: Posn end_;
  private: Posn start_;
  private: css::Style style_;

  public: Interval(const Interval& other);
  public: Interval(Posn start, Posn end);
  public: ~Interval();

  public: Posn end() const { return end_; }
  public: Posn start() const { return start_; }
  public: const css::Style& style() const { return style_; }
  public: void set_style(const css::Style& style);

  public: int Compare(const Interval* other) const;
  public: bool Contains(Posn offset) const;

  DISALLOW_ASSIGN(Interval);
};

}  // namespace text

#include <ostream>

namespace std {
ostream& operator<<(ostream& ostream, const text::Interval& interval);
ostream& operator<<(ostream& ostream, const text::Interval* interval);
}  // namespace std

#endif //!defined(INCLUDE_evita_text_interval_h)
