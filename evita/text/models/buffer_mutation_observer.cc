// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/models/buffer_mutation_observer.h"

#include "evita/text/models/offset.h"
#include "evita/text/models/static_range.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// BufferMutationObserver
//
BufferMutationObserver::BufferMutationObserver() {}

BufferMutationObserver::~BufferMutationObserver() {}

void BufferMutationObserver::DidChangeStyle(const StaticRange& range) {}
void BufferMutationObserver::DidDeleteAt(const StaticRange& range) {}
void BufferMutationObserver::DidInsertBefore(const StaticRange& range) {}
void BufferMutationObserver::WillDeleteAt(const StaticRange& range) {}

}  // namespace text
