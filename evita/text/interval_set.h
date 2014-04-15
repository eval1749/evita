// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_interval_set_h)
#define INCLUDE_evita_text_interval_set_h

#include <unordered_set>

#include "evita/ed_BinTree.h"
#include "evita/li_util.h"
#include "evita/text/buffer_mutation_observer.h"

namespace css {
class Style;
}

namespace text {

class Interval;

class IntervalSet : public BufferMutationObserver {
  private: typedef DoubleLinkedList_<Interval> IntervalList;
  private: typedef BinaryTree<Interval> IntervalTree;

  private: IntervalList list_;
  private: IntervalTree tree_;

  public: IntervalSet(Buffer* buffer);
  public: virtual ~IntervalSet();

  public: Interval* GetIntervalAt(Posn offset) const;
  private: void InsertAfter(Interval* interval, Interval* ref_interval);
  private: void InsertBefore(Interval* interval, Interval* ref_interval);
  private: void RemoveInterval(Interval* interval);
  public: void SetStyle(Posn, Posn, const css::Style& style_values);
  // Split |interval| at |offset| and return new interval starts at |offset|.
  private: Interval* SplitAt(Interval* interval, Posn offset);
  private: Interval* TryMergeInterval(Interval*);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void DidInsertBefore(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(IntervalSet);
};

}   // text

#endif //!defined(INCLUDE_evita_text_buffer_mutation_observer_h)
