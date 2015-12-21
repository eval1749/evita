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
#include "evita/text/offset.h"
#include "evita/text/range_set_base.h"
#include "evita/text/static_range.h"

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
  Interval* GetIntervalAt(Offset offset) const;

  // Returns true if style is changed for start until end, or false if nothing
  // is changed.
  void SetStyle(Offset start, Offset end, const css::Style& style_values);

  // Split |interval| at |offset| and return new interval starts at |offset|.
  Interval* SplitAt(Interval* interval, Offset offset);

 private:
  class Editor;

  // Merge |interval2| into |interval1| if possible and return true if merged.
  bool MergeAdjacentIntervalsIfPossible(Interval* interval1,
                                        Interval* interval2);

  void NotifyChange(const Interval& interval);

  // Try merge interval with adjacent intervals.
  Interval* TryMergeInterval(Interval* interval);

  // BufferMutationObserver
  void DidDeleteAt(const StaticRange& range) final;
  void DidInsertBefore(const StaticRange& range) final;

  Buffer* const buffer_;

  // Order |Interval| by end boundary point. |intervals_| always have at least
  // one |Interval|. The end boundary point of the last interval is length of
  // document plus one and represents default style.
  std::set<Interval*> intervals_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

//////////////////////////////////////////////////////////////////////
//
// IntervalSet::Impl::Editor
//
class IntervalSet::Impl::Editor final : public RangeSetBase {
 public:
  explicit Editor(Impl* interval_set);
  ~Editor();

  void Update(Interval* interval, Offset new_start, Offset new_end);

 private:
  struct UpdateOperation {
    Interval* interval;
    Offset start;
    Offset end;
  };

  Impl* const interval_set_;
  std::vector<Interval*> removes_;
  std::vector<UpdateOperation> updates_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

IntervalSet::Impl::Editor::Editor(Impl* interval_set)
    : interval_set_(interval_set) {}

IntervalSet::Impl::Editor::~Editor() {
  for (const auto& interval : removes_) {
    interval_set_->intervals_.erase(interval);
    delete interval;
  }
  for (const auto& update : updates_)
    set_range(update.interval, update.start, update.end);
}

void IntervalSet::Impl::Editor::Update(Interval* interval,
                                       Offset start,
                                       Offset end) {
  if (start == end)
    return removes_.push_back(interval);
  updates_.push_back({interval, start, end});
}

//////////////////////////////////////////////////////////////////////
//
// IntervalSet::Impl
//
IntervalSet::Impl::Impl(Buffer* buffer) : buffer_(buffer) {
  buffer_->AddObserver(this);

  // Insert default style.
  intervals_.insert(new Interval(Offset(), Offset(1)));
}

IntervalSet::Impl::~Impl() {
  buffer_->RemoveObserver(this);
  for (auto interval : intervals_) {
    delete interval;
  }
  intervals_.clear();
}

Interval* IntervalSet::Impl::GetIntervalAt(Offset offset) const {
  DCHECK(!intervals_.empty());
  if (offset == Offset(0))
    return *intervals_.begin();
  Interval interval(offset, offset + OffsetDelta(1));
  const auto& it = intervals_.lower_bound(&interval);
  DCHECK(intervals_.end() != it);
  DCHECK((*it)->Contains(offset));
  return *it;
}

bool IntervalSet::Impl::MergeAdjacentIntervalsIfPossible(Interval* interval1,
                                                         Interval* interval2) {
  DCHECK_EQ(interval1->end(), interval2->start());
  if (!CanMergeIntervals(interval1, interval2))
    return false;
  const auto end = interval2->end();
  const auto& it = intervals_.find(interval2);
  DCHECK(intervals_.end() != it);
  intervals_.erase(it);
  DCHECK(!intervals_.empty());
  delete interval2;
  set_range_end(interval1, end);
  return true;
}

void IntervalSet::Impl::NotifyChange(const Interval& interval) {
  static_cast<IntervalSetObserver*>(buffer_)
      ->DidChangeInterval(interval.start(), interval.end());
}

void IntervalSet::Impl::SetStyle(Offset start,
                                 Offset end,
                                 const css::Style& style) {
  DCHECK_LT(start, end);
  auto offset = start;
  while (offset < end) {
    const auto interval = GetIntervalAt(offset);
    DCHECK_LE(interval->start(), offset);
    const auto target =
        interval->start() == offset ? interval : SplitAt(interval, offset);
    if (target->end() == end) {
      if (style.IsSubsetOf(target->style()))
        return;
      target->set_style(style);
      NotifyChange(*target);
      TryMergeInterval(target);
      return;
    }

    if (target->end() > end) {
      if (style.IsSubsetOf(target->style()))
        return;
      SplitAt(target, end);
      target->set_style(style);
      NotifyChange(*target);
      TryMergeInterval(target);
      return;
    }

    if (style.IsSubsetOf(target->style())) {
      offset = target->end();
      continue;
    }
    target->set_style(style);
    NotifyChange(*target);
    offset = TryMergeInterval(target)->end();
  }
}

Interval* IntervalSet::Impl::SplitAt(Interval* interval, Offset offset) {
  DCHECK_GT(offset, interval->start());
  DCHECK_LT(offset, interval->end());
  const auto new_interval = new Interval(*interval);
  set_range_end(interval, offset);
  set_range_start(new_interval, offset);
  intervals_.insert(new_interval);
  return new_interval;
}

Interval* IntervalSet::Impl::TryMergeInterval(Interval* interval) {
  const auto& it = intervals_.lower_bound(interval);
  DCHECK(intervals_.end() != it);
  const auto& next_it = std::next(it);

  // Merge to previous
  if (it != intervals_.begin()) {
    const auto previous = *std::prev(it);
    if (MergeAdjacentIntervalsIfPossible(previous, interval))
      interval = previous;
  }

  // Absorb next
  if (next_it != intervals_.end()) {
    const auto next = *next_it;
    DCHECK_EQ(interval->end(), next->start());
    MergeAdjacentIntervalsIfPossible(interval, next);
  }

  return interval;
}

// BufferMutationObserver
void IntervalSet::Impl::DidDeleteAt(const StaticRange& range) {
  const auto delete_start = range.start();
  const auto length = range.length();
  const auto delete_end = range.end();
  Editor editor(this);
  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() > delete_start; ++it) {
    const auto interval = *it;
    const auto start = interval->start() > delete_start
                           ? interval->start() > delete_end
                                 ? interval->start() - length
                                 : delete_start
                           : interval->start();
    const auto end =
        interval->end() > delete_end ? interval->end() - length : delete_start;
    editor.Update(interval, start, end);
  }
}

void IntervalSet::Impl::DidInsertBefore(const StaticRange& range) {
  const auto offset = range.start();
  const auto length = range.length();
  for (auto it = intervals_.rbegin();
       it != intervals_.rend() && (*it)->end() >= offset; ++it) {
    auto interval = *it;
    if (interval->start() >= offset) {
      set_range(interval, interval->start() + length, interval->end() + length);
      continue;
    }
    set_range_end(interval, interval->end() + length);
  }

  if (offset.value())
    return;

  // Set default style to new text inserted at start of document.
  const auto head = *intervals_.begin();
  DCHECK_EQ(Offset(length.value()), head->start());
  // TODO(eval1749): We should check head interval has default style or not
  // without creating Interval object.
  const auto interval = new Interval(Offset(), head->start());
  if (CanMergeIntervals(interval, head)) {
    set_range_start(head, Offset());
    delete interval;
    return;
  }
  intervals_.insert(interval);
}

//////////////////////////////////////////////////////////////////////
//
// IntervalSet
//
IntervalSet::IntervalSet(Buffer* buffer) : impl_(new Impl(buffer)) {}

IntervalSet::~IntervalSet() {}

Interval* IntervalSet::GetIntervalAt(Offset offset) const {
  return impl_->GetIntervalAt(offset);
}

void IntervalSet::SetStyle(Offset start, Offset end, const css::Style& style) {
  impl_->SetStyle(start, end, style);
}

}  // namespace text
