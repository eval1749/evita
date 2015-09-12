// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LINE_NUMBER_CACHE_H_
#define EVITA_TEXT_LINE_NUMBER_CACHE_H_

#include <map>

#include "evita/text/buffer_mutation_observer.h"

namespace text {

class Buffer;

struct LineNumberAndOffset {
  int number;
  Posn offset;
};

class LineNumberCache final : public BufferMutationObserver {
 public:
  explicit LineNumberCache(Buffer* buffer);
  ~LineNumberCache() final;

  LineNumberAndOffset Get(Posn offset);

 private:
  // Invalidate cache entries after |offset|, inclusive.
  void InvalidateCache(Posn offset);

  // Scan buffer from |line_start_offset| which line number is |line_number|,
  // to |offset| and returns line number for |offset|.
  LineNumberAndOffset UpdateCache(Posn line_start_offset,
                                  int line_number,
                                  Posn offset);

  // BufferMutationObserver
  void DidDeleteAt(Posn offset, size_t length) final;
  void DidInsertAt(Posn offset, size_t length) final;

  Buffer* buffer_;
  std::map<Posn, int> map_;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCache);
};

}  // namespace text

#endif  // EVITA_TEXT_LINE_NUMBER_CACHE_H_
