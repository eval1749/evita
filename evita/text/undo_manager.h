// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_manager_h)
#define INCLUDE_evita_text_undo_manager_h

#include "base/strings/string16.h"

namespace text {

class Record;

// m_cb
// Holds hint of total size of edit log records.
// m_fMerge
// We merge consecutive insert/delete edit log records.
// m_fTruncate
// We truncate edit log if undo/redo operation is interrupted.
//
class UndoManager {
  public: enum State {
    State_Disabled,
    State_Log,
    State_Redo,
    State_Undo,
  };

  private: size_t m_cb;
  private: State m_eState;
  private: bool m_fMerge;
  private: bool m_fTruncate;
  private: HANDLE m_hObjHeap;
  private: Buffer* m_pBuffer;
  private: Record* m_pFirst;
  private: Record* m_pLast;
  private: Record* m_pRedo;
  private: Record* m_pUndo;

  public: explicit UndoManager(Buffer*);
  public: ~UndoManager();

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
  public: size_t GetSize() const { return m_cb; }

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

  DISALLOW_COPY_AND_ASSIGN(UndoManager);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_undo_manager_h)
