// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_stack_h)
#define INCLUDE_evita_text_undo_stack_h

#include "base/strings/string16.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

class UndoStep;

class UndoStack : public BufferMutationObserver {
  public: enum State {
    State_Disabled,
    State_Log,
    State_Redo,
    State_Undo,
  };

  private: State m_eState;
  private: HANDLE m_hObjHeap;
  private: Buffer* m_pBuffer;
  private: UndoStep* m_pFirst;
  private: UndoStep* m_pLast;
  private: UndoStep* m_pRedo;
  private: UndoStep* m_pUndo;

  public: explicit UndoStack(Buffer*);
  public: ~UndoStack();

  // [A]
  public: void* Alloc(size_t);
  private: void addUndoStep(UndoStep*);

  // [B]
  public: void BeginUndoGroup(const base::string16& name);

  // [C]
  public: bool CanRedo() const;
  public: bool CanUndo() const;
  public: void CheckPoint();

  // [D]
  private: void delUndoStep(UndoStep*);
  private: void discardUndoStep(UndoStep*);

  // [E]
  public: void Empty();
  public: void EndUndoGroup(const base::string16& name);

  // [F]
  public: void Free(void*);

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }

  // [P]
  public: void PushDeleteText(Posn, Posn);
  public: void PushInsertText(Posn, Posn);

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
