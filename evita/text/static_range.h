// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STATIC_RANGE_H_
#define EVITA_TEXT_STATIC_RANGE_H_

#include "base/logging.h"
#include "base/macros.h"
#include "evita/base/memory/allocator.h"
#include "evita/text/offset.h"

namespace text {

class Buffer;

//////////////////////////////////////////////////////////////////////
//
// StaticRange
//
class StaticRange final {
  STACK_ALLOCATED();

 public:
  StaticRange(const Buffer& buffer, Offset start, Offset end);
  ~StaticRange();

  OffsetDelta length() const { return end() - start(); }

#if DCHECK_IS_ON()
  Offset end() const;
  Offset start() const;
#else
  Offset end() const { return end_; }
  Offset start() const { return start_; }
#endif

 private:
#if DCHECK_IS_ON()
  const Buffer& buffer_;
  const int buffer_revision_;
#endif

  const text::Offset end_;
  const text::Offset start_;

  DISALLOW_COPY_AND_ASSIGN(StaticRange);
};

}  // namespace text

#endif  // EVITA_TEXT_STATIC_RANGE_H_
