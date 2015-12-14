// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_SCOPED_UNDO_GROUP_H_
#define EVITA_TEXT_SCOPED_UNDO_GROUP_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace text {

class Buffer;
class Range;

/// <summary>
/// Utility class for inserting begin/end undo operation. Instances are
/// created on stack instead of heap.
/// </summary>
class ScopedUndoGroup final {
 public:
  ScopedUndoGroup(Range* range, const base::string16& name);
  ScopedUndoGroup(Buffer* buffer, const base::string16& name);
  ~ScopedUndoGroup();

 private:
  Buffer* const buffer_;
  const base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(ScopedUndoGroup);
};

}  // namespace text

#endif  // EVITA_TEXT_SCOPED_UNDO_GROUP_H_
