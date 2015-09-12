// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_H_
#define EVITA_TEXT_RANGE_H_

#include "base/strings/string16.h"
#include "evita/precomp.h"

namespace text {

class Buffer;
class RangeSet;

class Range {
 public:
  Range(Buffer* buffer, Posn start, Posn end);
  virtual ~Range();

  Buffer* buffer() const { return buffer_; }
  Posn end() const { return end_; }
  void set_end(Posn new_end);
  Posn start() const { return start_; }
  void set_start(Posn new_start);
  base::string16 text() const;
  void set_text(const base::string16& new_text);

  void SetRange(Posn start, Posn end);

 protected:
  virtual void DidChangeRange();

 private:
  friend class RangeSet;

  Posn EnsureOffset(Posn offset) const;

  Buffer* buffer_;
  Posn end_;
  Posn start_;

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_H_
