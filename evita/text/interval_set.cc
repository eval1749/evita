// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/interval_set.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/ed_interval.h"

namespace text {

IntervalSet::IntervalSet(Buffer* buffer) {
  buffer->AddObserver(this);

  {
    auto const interval = new Interval(0, 1);
    interval->SetStyle(&g_DefaultStyle);
    tree_.Insert(interval);
    list_.Append(interval);
  }
}

IntervalSet::~IntervalSet() {
  while (auto const interval = list_.GetFirst()) {
    list_.Delete(interval);
    delete interval;
  }
}

Interval* IntervalSet::GetIntervalAt(Posn offset) const {
  auto runner = tree_.GetRoot();
  while (runner) {
    if (runner->Contains(offset))
        break;
    runner = offset < runner->GetStart() ? runner = runner->GetLeft() :
                                           runner = runner->GetRight();
  }
  DCHECK(runner);
  return runner;
}

void IntervalSet::InsertAfter(Interval* interval, Interval* ref) {
  list_.InsertAfter(interval, ref);
  tree_.Insert(interval);
}

void IntervalSet::InsertBefore(Interval* interval, Interval* ref) {
  list_.InsertBefore(interval, ref);
  tree_.Insert(interval);
}

void IntervalSet::RemoveInterval(Interval* interval) {
    list_.Delete(interval);
    tree_.Delete(interval);
}

// BufferMutationObserver
void IntervalSet::DidDeleteAt(int offset, size_t text_length) {
  {
    auto interval = list_.GetLast();
    while (interval && interval->m_lEnd > offset) {
      auto const next = interval->GetPrev();
      auto const start = interval->m_lStart > offset ?
          std::max(static_cast<Posn>(interval->m_lStart - text_length),
                   static_cast<Posn>(offset)) : interval->m_lStart;
      auto const end =
        std::max(static_cast<Posn>(interval->m_lEnd - text_length),
                 static_cast<Posn>(offset));
      if (start == end) {
        list_.Delete(interval);
        tree_.Delete(interval);
      }
      interval = next;
    }
  }

  auto interval = list_.GetLast();
  while (interval && interval->m_lEnd > offset) {
    if (interval->m_lStart > offset) {
      interval->m_lStart =
          std::max(static_cast<Posn>(interval->m_lStart - text_length),
                   static_cast<Posn>(offset));
    }
    interval->m_lEnd =
        std::max(static_cast<Posn>(interval->m_lEnd - text_length),
                 static_cast<Posn>(offset));
    interval = interval->GetPrev();
  }
}

void IntervalSet::DidInsertAt(int offset, size_t text_length) {
  auto interval = list_.GetLast();
  while (interval && interval->m_lEnd > offset) {
    if (interval->m_lStart > offset)
      interval->m_lStart += text_length;
    interval->m_lEnd += text_length;
    interval = interval->GetPrev();
  }
}

void IntervalSet::DidInsertBefore(int offset, size_t text_length) {
  auto interval = list_.GetLast();
  while (interval && interval->m_lEnd >= offset) {
    if (interval->m_lStart >= offset)
      interval->m_lStart += text_length;
    interval->m_lEnd += text_length;
    interval = interval->GetPrev();
  }

  if (!offset) {
    // Set default style to new text inserted at start of document.
    auto const head = list_.GetFirst();
    DCHECK_EQ(static_cast<Posn>(text_length), head->GetStart());
    // TODO(yosi) We should check head interval has default style or not
    // without creating Interval object.
    auto const interval = new Interval(0, head->GetStart());
    if (interval->CanMerge(head)) {
      head->m_lStart = 0;
      delete interval;
    } else {
      list_.Prepend(interval);
      tree_.Insert(interval);
    }
  }
}

}  // namespace text
