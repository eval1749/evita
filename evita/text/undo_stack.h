// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_stack_h)
#define INCLUDE_evita_text_undo_stack_h

#include <vector>

#include "base/strings/string16.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

class UndoStep;

class UndoStack : public BufferMutationObserver {
  public: enum class State {
    Normal,
    Redo,
    Undo,
  };

  private: Buffer* buffer_;
  private: std::vector<UndoStep*> redo_steps_;
  private: State state_;
  private: std::vector<UndoStep*> undo_steps_;

  public: explicit UndoStack(Buffer*);
  public: virtual ~UndoStack();

  // [B]
  public: void BeginUndoGroup(const base::string16& name);

  // [C]
  public: bool CanRedo() const;
  public: bool CanUndo() const;
  public: void Clear();

  // [E]
  public: void EndUndoGroup(const base::string16& name);

  // [R]
  public: Posn Redo(Posn, Count);

  // [U]
  public: Posn Undo(Posn, Count);

  // BufferMutationObserver
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void WillDeleteAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(UndoStack);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_undo_stack_h)
