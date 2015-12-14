// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LINE_NUMBER_CACHE_H_
#define EVITA_TEXT_LINE_NUMBER_CACHE_H_

#include <map>

#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/offset.h"

namespace text {

class Buffer;

struct LineNumberAndOffset {
  int number;
  Offset offset;
};

class LineNumberCache final : public BufferMutationObserver {
 public:
  explicit LineNumberCache(Buffer* buffer);
  ~LineNumberCache() final;

  LineNumberAndOffset Get(Offset offset);

 private:
  // Invalidate cache entries after |offset|, inclusive.
  void InvalidateCache(Offset offset);

  // Scan buffer from |line_start_offset| which line number is |line_number|,
  // to |offset| and returns line number for |offset|.
  LineNumberAndOffset UpdateCache(Offset line_start_offset,
                                  int line_number,
                                  Offset offset);

  // BufferMutationObserver
  void DidDeleteAt(Offset offset, OffsetDelta length) final;
  void DidInsertBefore(Offset offset, OffsetDelta length) final;

  Buffer* buffer_;
  std::map<Offset, int> map_;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCache);
};

}  // namespace text

#endif  // EVITA_TEXT_LINE_NUMBER_CACHE_H_
