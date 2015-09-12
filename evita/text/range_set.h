// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_SET_H_
#define EVITA_TEXT_RANGE_SET_H_

#include <unordered_set>

#include "evita/text/buffer_mutation_observer.h"

namespace text {

class Range;

class RangeSet final : public BufferMutationObserver {
 public:
  explicit RangeSet(Buffer* buffer);
  ~RangeSet() final;

  void AddRange(Range* range);
  void RemoveRange(Range* range);

 private:
  // BufferMutationObserver
  void DidDeleteAt(Posn offset, size_t length) final;
  void DidInsertAt(Posn offset, size_t length) final;
  void DidInsertBefore(Posn offset, size_t length) final;

  std::unordered_set<Range*> ranges_;

  DISALLOW_COPY_AND_ASSIGN(RangeSet);
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_SET_H_
