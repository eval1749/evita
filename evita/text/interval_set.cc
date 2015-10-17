// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/interval_set.h"

#include <algorithm>
#include <functional>
#include <set>
#include <vector>

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/range_set_base.h"

namespace std {
template <>
struct less<text::Interval*> {
  bool operator()(const text::Interval* x, const text::Interval* y) const {
    return x->end() < y->end();
  }
};
}  // namespace std

namespace text {

namespace {
// Returns true if |interval1| and |interval12| can be merged. If two
// intervals are mergeable, both intervals have equivalent styles.
bool CanMergeIntervals(const Interval* interval1, const Interval* interval2) {
  return interval1->style() == interval2->style();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IntervalSet::Impl
//
class IntervalSet::Impl final : public RangeSetBase,
                                public BufferMutationObserver {
 public:
  explicit Impl(Buffer* buffer);
  virtual ~Impl();

  // Get |Interval| contains |offset|.
  Interval* GetIntervalAt(Posn offset) const;

  // Set style on specified range.
  void set_style(Posn start, Posn end, const css::Style& style_values);

  // Split |interval| at |offset| and return new interval starts at |offset|.
  Interval* SplitAt(Interval* interval, Posn offset);

 private:
  // Merge |interval2| into |interval1| if possible and return true if merged.
  bool MergeAdjacentIntervalsIfPossible(Interval* interval1,
                                        Interval* interval2);

  // Try merge interval with adjacent intervals.
  Interval* TryMergeInterval(Interval* interval);

  // BufferMutationObserver
  void DidDeleteAt(Posn offset, size_t length) override;
  void DidInsertAt(Posn offset, size_t length) override;
  void DidInsertBefore(Posn offset, size_t length) override;

  Buffer* const buffer_;

  // Order |Interval| by end boundary point. |intervals_| always have at least
  // one |Interval|. The end boundary point of the last interval is length of
  // document plus one and represents default style.
  std::set<Interval*> intervals_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

IntervalSet::Impl::Impl(Buffer* buffer) : buffer_(buffer) {
  buffer_->AddObserver(this);

  // Insert default style.
  intervals_.insert(new Interval(0, 1));
}

IntervalSet::Impl::~Impl() {
  buffer_->RemoveObserver(this);
  for (auto interval : intervals_) {
    delete interval;
  }
  intervals_.clear();
}

Interval* IntervalSet::Impl::GetIntervalAt(Posn offset) const {
  DCHECK(!intervals_.empty());
  if (!offset)
    return *intervals_.begin();
  Interval interval(offset, offset + 1);
  auto const it = intervals_.lower_bound(&interval);
  DCHECK(intervals_.end() != it);
  DCHECK((*it)->Contains(offset));
  return *it;
}

bool IntervalSet::Impl::MergeAdjacentIntervalsIfPossible(Interval* interval1,
                                                         Interval* interval2) {
  DCHECK_EQ(interval1->end(), interval2->start());
  if (!CanMergeIntervals(interval1, interval2))
    return false;
  auto const end = interval2->end();
  auto const it = intervals_.find(interval2);
  DCHECK(intervals_.end() != it);
  intervals_.erase(it);
  DCHECK(!intervals_.empty());
  delete interval2;
  set_range_end(interval1, end);
  return true;
}

void IntervalSet::Impl::set_style(Posn start,
                                  Posn end,
                                  const css::Style& style) {
  DCHECK_LT(start, end);
  auto offset = start;
  while (offset < end) {
    auto const interval = GetIntervalAt(offset);
    DCHECK_LE(interval->start(), offset);
    auto const target =
        interval->start() == offset ? interval : SplitAt(interval, offset);
    if (target->end() == end) {
      target->set_style(style);
      TryMergeInterval(target);
      break;
    }

    if (target->end() > end) {
      SplitAt(target, end);
      target->set_style(style);
      TryMergeInterval(target);
      break;
    }

    target->set_style(style);
    offset = TryMergeInterval(target)->end();
  }
}

Interval* IntervalSet::Impl::SplitAt(Interval* interval, Posn offset) {
  DCHECK_GT(offset, interval->start());
  DCHECK_LT(offset, interval->end());
  auto const new_interval = new Interval(*interval);
  set_range_end(interval, offset);
  set_range_start(new_interval, offset);
  intervals_.insert(new_interval);
  return new_interval;
}

Interval* IntervalSet::Impl::TryMergeInterval(Interval* interval) {
  auto it = intervals_.lower_bound(interval);
  DCHECK(intervals_.end() != it);

  auto previous_it = it;
  auto next_it = it;
  ++next_it;

  // Merge to previous
  if (previous_it != intervals_.begin()) {
    --previous_it;
    auto const previous = *previous_it;
    if (MergeAdjacentIntervalsIfPossible(previous, interval))
      interval = previous;
  }

  // Absorb next
  if (next_it != intervals_.end()) {
    auto const next = *next_it;
    DCHECK_EQ(interval->end(), next->start());
    MergeAdjacentIntervalsIfPossible(interval, next);
  }

  return interval;
}

// BufferMutationObserver
void IntervalSet::Impl::DidDeleteAt(Posn offset, size_t length) {
  std::vector<Interval*> intervals_to_remove;
  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() > offset; ++it) {
    auto const interval = *it;
    auto const start =
        interval->start() > offset
            ? std::max(static_cast<Posn>(interval->start() - length), offset)
            : interval->start();
    auto const end =
        std::max(static_cast<Posn>(interval->end() - length), offset);
    if (start == end)
      intervals_to_remove.push_back(*it);
  }
  DCHECK_LT(intervals_to_remove.size(), intervals_.size());

  for (auto interval : intervals_to_remove) {
    intervals_.erase(interval);
    delete interval;
  }
  DCHECK(!intervals_.empty());

  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() > offset; ++it) {
    auto const interval = *it;
    auto const start =
        interval->start() > offset
            ? std::max(static_cast<Posn>(interval->start() - length), offset)
            : interval->start();
    auto const end =
        std::max(static_cast<Posn>(interval->end() - length), offset);
    set_range(interval, start, end);
  }
}

void IntervalSet::Impl::DidInsertAt(Posn offset, size_t length) {
  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() > offset; ++it) {
    auto interval = *it;
    if (interval->start() > offset) {
      set_range(interval, static_cast<Posn>(interval->start() + length),
                static_cast<Posn>(interval->end() + length));
    } else {
      set_range_end(interval, static_cast<Posn>(interval->end() + length));
    }
  }
}

void IntervalSet::Impl::DidInsertBefore(Posn offset, size_t length) {
  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() >= offset; ++it) {
    auto interval = *it;
    if (interval->start() >= offset) {
      set_range(interval, static_cast<Posn>(interval->start() + length),
                static_cast<Posn>(interval->end() + length));
    } else {
      set_range_end(interval, static_cast<Posn>(interval->end() + length));
    }
  }

  if (!offset) {
    // Set default style to new text inserted at start of document.
    auto const head = *intervals_.begin();
    DCHECK_EQ(static_cast<Posn>(length), head->start());
    // TODO(eval1749): We should check head interval has default style or not
    // without creating Interval object.
    auto const interval = new Interval(0, head->start());
    if (CanMergeIntervals(interval, head)) {
      set_range_start(head, 0);
      delete interval;
    } else {
      intervals_.insert(interval);
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// IntervalSet
IntervalSet::IntervalSet(Buffer* buffer) : impl_(new Impl(buffer)) {}

IntervalSet::~IntervalSet() {}

Interval* IntervalSet::GetIntervalAt(Posn offset) const {
  return impl_->GetIntervalAt(offset);
}

void IntervalSet::SetStyle(Posn start, Posn end, const css::Style& style) {
  return impl_->set_style(start, end, style);
}

}  // namespace text
