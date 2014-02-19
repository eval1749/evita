// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_list_h)
#define INCLUDE_evita_text_range_list_h

#include <unordered_set>

#include "evita/text/buffer_mutation_observer.h"

namespace text {

class Range;

class RangeList : public BufferMutationObserver {
  private: std::unordered_set<Range*> ranges_;

  public: RangeList(Buffer* buffer);
  public: virtual ~RangeList();

  public: void AddRange(Range* range);
  public: void RemoveRange(Range* range);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void DidInsertBefore(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(RangeList);
};

}   // text

#endif //!defined(INCLUDE_evita_text_range_list_h)
