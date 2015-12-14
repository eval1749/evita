// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/buffer_mutation_observer.h"

#include "evita/text/offset.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// BufferMutationObserver
//
BufferMutationObserver::BufferMutationObserver() {}

BufferMutationObserver::~BufferMutationObserver() {}

void BufferMutationObserver::DidChangeStyle(Offset, OffsetDelta) {}

void BufferMutationObserver::DidDeleteAt(Offset, OffsetDelta) {}

void BufferMutationObserver::DidInsertAt(Offset, OffsetDelta) {}

void BufferMutationObserver::DidInsertBefore(Offset offset,
                                             OffsetDelta length) {
  DidInsertAt(offset, length);
}

void BufferMutationObserver::WillDeleteAt(Offset, OffsetDelta) {}

//////////////////////////////////////////////////////////////////////
//
// BufferMutationObservee
//
BufferMutationObservee::BufferMutationObservee() {}

BufferMutationObservee::~BufferMutationObservee() {}

}  // namespace text
