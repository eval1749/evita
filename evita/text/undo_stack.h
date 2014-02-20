// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_stack_h)
#define INCLUDE_evita_text_undo_stack_h

#include "base/strings/string16.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

class Record;

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
  private: Record* m_pFirst;
  private: Record* m_pLast;
  private: Record* m_pRedo;
  private: Record* m_pUndo;

  public: explicit UndoStack(Buffer*);
  public: ~UndoStack();

  // [A]
  public: void* Alloc(size_t);
  private: void addRecord(Record*);

  // [C]
  public: bool CanRedo() const;
  public: bool CanUndo() const;
  public: void CheckPoint();

  // [D]
  private: void delRecord(Record*);
  private: void discardRecord(Record*);


  // [E]
  public: void Empty();

  // [F]
  public: void Free(void*);

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }

  // [R]
  public: void RecordBegin(const base::string16& name);
  public: void RecordDelete(Posn, Posn);
  public: void RecordEnd(const base::string16& name);
  public: void RecordInsert(Posn, Posn);
  public: Posn Redo(Posn, Count);

  // [T]
  public: void TruncateLog();

  // [U]
  public: Posn Undo(Posn, Count);

  // BufferMutationObserver
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void WillDeleteAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(UndoStack);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_undo_stack_h)
