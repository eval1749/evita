// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/interval_set.h"

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/ed_BinTree.h"
#include "evita/li_util.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"

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
class IntervalSet::Impl : public BufferMutationObserver {
  private: typedef DoubleLinkedList_<Interval> IntervalList;
  private: typedef BinaryTree<Interval> IntervalTree;

  private: Buffer* buffer_;
  private: IntervalList list_;
  private: IntervalTree tree_;

  public: Impl(Buffer* buffer);
  public: virtual ~Impl();

  public: Interval* GetIntervalAt(Posn offset) const;
  private: void InsertAfter(Interval* interval, Interval* ref_interval);
  private: void InsertBefore(Interval* interval, Interval* ref_interval);
  private: void RemoveInterval(Interval* interval);
  public: void set_style(Posn, Posn, const css::Style& style_values);
  // Split |interval| at |offset| and return new interval starts at |offset|.
  private: Interval* SplitAt(Interval* interval, Posn offset);
  private: Interval* TryMergeInterval(Interval*);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void DidInsertBefore(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

IntervalSet::Impl::Impl(Buffer* buffer) : buffer_(buffer) {
  buffer_->AddObserver(this);

  {
    auto const interval = new Interval(0, 1);
    interval->set_style(*css::Style::Default());
    tree_.Insert(interval);
    list_.Append(interval);
  }
}

IntervalSet::Impl::~Impl() {
  buffer_->RemoveObserver(this);
  while (auto const interval = list_.GetFirst()) {
    list_.Delete(interval);
    delete interval;
  }
}

Interval* IntervalSet::Impl::GetIntervalAt(Posn offset) const {
  auto runner = tree_.GetRoot();
  while (runner) {
    if (runner->Contains(offset))
        break;
    runner = offset < runner->start() ? runner = runner->GetLeft() :
                                           runner = runner->GetRight();
  }
  DCHECK(runner);
  return runner;
}

void IntervalSet::Impl::InsertAfter(Interval* interval, Interval* ref) {
  list_.InsertAfter(interval, ref);
  tree_.Insert(interval);
}

void IntervalSet::Impl::InsertBefore(Interval* interval, Interval* ref) {
  list_.InsertBefore(interval, ref);
  tree_.Insert(interval);
}

void IntervalSet::Impl::RemoveInterval(Interval* interval) {
  list_.Delete(interval);
  tree_.Delete(interval);
}

void IntervalSet::Impl::set_style(Posn start, Posn end,
                                 const css::Style& style) {
  DCHECK_LT(start, end);
  auto offset = start;
  while (offset < end) {
    auto const interval = GetIntervalAt(offset);
    DCHECK_LE(interval->start(), offset);
    auto const target = interval->start() == offset ? interval :
        SplitAt(interval, offset);
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
  interval->end_ = offset;
  new_interval->start_ = offset;
  InsertAfter(new_interval, interval);
  return new_interval;
}

Interval* IntervalSet::Impl::TryMergeInterval(Interval* pIntv) {
  // Merge to previous
  if (Interval* pPrev = pIntv->GetPrev()) {
    DCHECK_EQ(pPrev->end(), pIntv->start());
    DCHECK_EQ(pPrev->GetNext(), pIntv);

    if (CanMergeIntervals(pIntv, pPrev)) {
      auto const lEnd = pIntv->end();
      RemoveInterval(pIntv);
      delete pIntv;
      pIntv = pPrev;
      pIntv->end_ = lEnd;
    }
  }

  // Absorb next
  if (Interval* pNext = pIntv->GetNext()) {
    DCHECK_EQ(pIntv->end(), pNext->start());
    if (CanMergeIntervals(pIntv, pNext)) {
      auto const lEnd = pNext->end();
      RemoveInterval(pNext);
      delete pNext;
      pIntv->end_ = lEnd;
    }
  }

  return pIntv;
}

// BufferMutationObserver
void IntervalSet::Impl::DidDeleteAt(Posn offset, size_t length) {
  {
    auto interval = list_.GetLast();
    while (interval && interval->end_ > offset) {
      auto const next = interval->GetPrev();
      auto const start = interval->start_ > offset ?
          std::max(static_cast<Posn>(interval->start_ - length),
                   offset) : interval->start_;
      auto const end =
        std::max(static_cast<Posn>(interval->end_ - length), offset);
      if (start == end) {
        list_.Delete(interval);
        tree_.Delete(interval);
      }
      interval = next;
    }
  }

  auto interval = list_.GetLast();
  while (interval && interval->end_ > offset) {
    if (interval->start_ > offset) {
      interval->start_ =
          std::max(static_cast<Posn>(interval->start_ - length),
                   offset);
    }
    interval->end_ = std::max(static_cast<Posn>(interval->end_ - length),
                                offset);
    interval = interval->GetPrev();
  }
}

void IntervalSet::Impl::DidInsertAt(Posn offset, size_t length) {
  auto interval = list_.GetLast();
  while (interval && interval->end_ > offset) {
    if (interval->start_ > offset)
      interval->start_ += length;
    interval->end_ += length;
    interval = interval->GetPrev();
  }
}

void IntervalSet::Impl::DidInsertBefore(Posn offset, size_t length) {
  auto interval = list_.GetLast();
  while (interval && interval->end_ >= offset) {
    if (interval->start_ >= offset)
      interval->start_ += length;
    interval->end_ += length;
    interval = interval->GetPrev();
  }

  if (!offset) {
    // Set default style to new text inserted at start of document.
    auto const head = list_.GetFirst();
    DCHECK_EQ(static_cast<Posn>(length), head->start());
    // TODO(yosi) We should check head interval has default style or not
    // without creating Interval object.
    auto const interval = new Interval(0, head->start());
    if (CanMergeIntervals(interval, head)) {
      head->start_ = 0;
      delete interval;
    } else {
      list_.Prepend(interval);
      tree_.Insert(interval);
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// IntervalSet
IntervalSet::IntervalSet(Buffer* buffer) : impl_(new Impl(buffer)) {
}

IntervalSet::~IntervalSet() {
}

Interval* IntervalSet::GetIntervalAt(Posn offset) const {
  return impl_->GetIntervalAt(offset);
}

void IntervalSet::SetStyle(Posn start, Posn end, const css::Style& style) {
  return impl_->set_style(start, end, style);
}

}  // namespace text
