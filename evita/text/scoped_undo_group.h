// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_scoped_undo_group_h)
#define INCLUDE_evita_text_scoped_undo_group_h

namespace text {

/// <summary>
/// Utility class for inserting begin/end undo operation. Instances are
/// created on stack instead of heap.
/// </summary>
class ScopedUndoGroup {
  private: Buffer* buffer_;
  private: const base::string16 name_;

  public: ScopedUndoGroup(Range* range, const base::string16& name);
  public: ScopedUndoGroup(Buffer* buffer, const base::string16& name);
  public: ~ScopedUndoGroup();

  DISALLOW_COPY_AND_ASSIGN(ScopedUndoGroup);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_scoped_undo_group_h)
