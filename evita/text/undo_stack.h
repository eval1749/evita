// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_UNDO_STACK_H_
#define EVITA_TEXT_UNDO_STACK_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

class UndoStep;

class UndoStack final : public BufferMutationObserver {
 public:
  enum class State {
    Normal,
    Redo,
    Undo,
  };

  explicit UndoStack(Buffer* buffer);
  ~UndoStack() final;

  void BeginUndoGroup(const base::string16& name);
  bool CanRedo() const;
  bool CanUndo() const;
  void Clear();
  void EndUndoGroup(const base::string16& name);
  Offset Redo(Offset offset, int count);
  Offset Undo(Offset offset, int count);

 private:
  // BufferMutationObserver
  void DidInsertAt(Offset offset, OffsetDelta length) final;
  void WillDeleteAt(Offset offset, OffsetDelta length) final;

  Buffer* buffer_;
  std::vector<UndoStep*> redo_steps_;
  State state_;
  std::vector<UndoStep*> undo_steps_;

  DISALLOW_COPY_AND_ASSIGN(UndoStack);
};

}  // namespace text

#endif  // EVITA_TEXT_UNDO_STACK_H_
