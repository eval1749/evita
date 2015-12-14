// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_RANGE_H_
#define EVITA_TEXT_RANGE_H_

#include "base/strings/string16.h"
#include "evita/text/offset.h"

namespace text {

class Buffer;
class RangeSet;

//////////////////////////////////////////////////////////////////////
//
// Range
//
class Range {
 public:
  Range(Buffer* buffer, Offset start, Offset end);
  virtual ~Range();

  Buffer* buffer() const { return buffer_; }
  Offset end() const { return end_; }
  void set_end(Offset new_end);
  Offset start() const { return start_; }
  void set_start(Offset new_start);
  base::string16 text() const;
  void set_text(const base::string16& new_text);

  void SetRange(Offset start, Offset end);

 protected:
  virtual void DidChangeRange();

 private:
  friend class RangeSet;

  Offset EnsureOffset(Offset offset) const;

  Buffer* buffer_;
  Offset end_;
  Offset start_;

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace text

#endif  // EVITA_TEXT_RANGE_H_
