// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/buffer_mutation_observer.h"

namespace text {

BufferMutationObserver::BufferMutationObserver() {
}

BufferMutationObserver::~BufferMutationObserver() {
}

void BufferMutationObserver::DidChangeStyle(Posn, size_t) {
}

void BufferMutationObserver::DidDeleteAt(Posn, size_t) {
}

void BufferMutationObserver::DidInsertAt(Posn, size_t) {
}

void BufferMutationObserver::DidInsertBefore(Posn offset, size_t length) {
  DidInsertAt(offset, length);
}

void BufferMutationObserver::WillDeleteAt(Posn, size_t) {
}

}  // namespace text
