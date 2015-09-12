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
  Posn Redo(Posn offset, Count count);
  Posn Undo(Posn offset, Count count);

 private:
  // BufferMutationObserver
  void DidInsertAt(Posn offset, size_t length) override;
  void WillDeleteAt(Posn offset, size_t length) override;

  Buffer* buffer_;
  std::vector<UndoStep*> redo_steps_;
  State state_;
  std::vector<UndoStep*> undo_steps_;

  DISALLOW_COPY_AND_ASSIGN(UndoStack);
};

}  // namespace text

#endif  // EVITA_TEXT_UNDO_STACK_H_
