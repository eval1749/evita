// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_h)
#define INCLUDE_evita_text_range_h

#include "base/strings/string16.h"

namespace text {

class RangeSet;

class Range {
  friend class RangeSet;

  private: Buffer* buffer_;
  private: Posn end_;
  private: Posn start_;

  public: Range(Buffer* buffer, Posn start, Posn end);
  public: ~Range();

  public: Buffer* buffer() const { return buffer_; }
  public: Posn end() const { return end_; }
  public: void set_end(Posn new_end);
  public: Posn start() const { return start_; }
  public: void set_start(Posn new_start);
  public: base::string16 text() const;
  public: void set_text(const base::string16& new_text);

  private: Posn EnsureOffset(Posn offset) const;
  public: void SetRange(Posn start, Posn end);

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_range_h)
