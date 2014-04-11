// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_line_number_cache_h)
#define INCLUDE_evita_text_line_number_cache_h

#include <map>

#include "evita/text/buffer_mutation_observer.h"

namespace text {

class Buffer;

struct LineNumberAndOffset{
  int number;
  Posn offset;
};


class LineNumberCache : public BufferMutationObserver {
  private: Buffer* buffer_;
  private: std::map<Posn, int> map_;

  public: LineNumberCache(Buffer* buffer);
  public: ~LineNumberCache();

  public: LineNumberAndOffset Get(Posn offset);

  // Invalidate cache entries after |offset|, inclusive.
  private: void InvalidateCache(Posn offset);

  // Scan buffer from |line_start_offset| which line number is |line_number|,
  // to |offset| and returns line number for |offset|.
  private: LineNumberAndOffset UpdateCache(Posn line_start_offset,
                                           int line_number, Posn offset);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCache);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_line_number_cache_h)
