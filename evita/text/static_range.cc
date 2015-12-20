// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/static_range.h"

#include "evita/text/buffer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// StaticRange
//
#if DCHECK_IS_ON()
StaticRange::StaticRange(const Buffer& buffer, Offset start, Offset end)
    : buffer_(buffer),
      buffer_revision_(buffer.revision()),
      end_(end),
      start_(start) {
  const_cast<Buffer&>(buffer_).RegisterStaticRange(*this);
}

StaticRange::~StaticRange() {
  const_cast<Buffer&>(buffer_).UnregisterStaticRange(*this);
}

Offset StaticRange::end() const {
  DCHECK_EQ(buffer_.revision(), buffer_revision_);
  return end_;
}

Offset StaticRange::start() const {
  DCHECK_EQ(buffer_.revision(), buffer_revision_);
  return start_;
}
#else
StaticRange::StaticRange(const Buffer& buffer, Offset start, Offset end)
    : end_(end), start_(start) {}

StaticRange::~StaticRange() {}
#endif

}  // namespace text
