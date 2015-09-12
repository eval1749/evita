// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/scoped_undo_group.h"

#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/text/undo_stack.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ScopedUndoGroup
//
ScopedUndoGroup::ScopedUndoGroup(Buffer* buffer, const base::string16& name)
    : buffer_(buffer), name_(name) {
  buffer->StartUndoGroup(name_);
}

ScopedUndoGroup::ScopedUndoGroup(Range* range, const base::string16& name)
    : ScopedUndoGroup(range->buffer(), name) {}

ScopedUndoGroup::~ScopedUndoGroup() {
  buffer_->EndUndoGroup(name_);
}

}  // namespace text
