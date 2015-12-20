// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_UNDO_STACK_H_
#define EVITA_TEXT_UNDO_STACK_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

class StaticRange;
class UndoStep;

//////////////////////////////////////////////////////////////////////
//
// UndoStack
//
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
  void DidInsertBefore(const StaticRange& range) final;
  void WillDeleteAt(const StaticRange& range) final;

  Buffer* const buffer_;
  std::vector<std::unique_ptr<UndoStep>> redo_steps_;
  State state_;
  std::vector<std::unique_ptr<UndoStep>> undo_steps_;

  DISALLOW_COPY_AND_ASSIGN(UndoStack);
};

}  // namespace text

#endif  // EVITA_TEXT_UNDO_STACK_H_
