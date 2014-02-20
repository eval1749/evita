// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/undo_stack.h"

#include "base/logging.h"
#include "common/castable.h"
#include "evita/text/buffer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Record
//
class Record : common::Castable {
  DECLARE_CASTABLE_CLASS(Record, Castable);

  friend class UndoStack;

  public: void* operator new(size_t cb, UndoStack* p) {
    return p->Alloc(cb);
  }

  private: void operator delete(void*) {
  }

  protected: Record* m_pNext;
  protected: Record* m_pPrev;

  protected: Record();
  public: virtual ~Record() = default;

  public: virtual void Discard(UndoStack*) {}
  public: virtual Posn GetAfterRedo() const = 0;
  public: virtual Posn GetAfterUndo() const = 0;
  public: virtual Posn GetBeforeRedo() const = 0;
  public: virtual Posn GetBeforeUndo() const = 0;
  public: virtual void Redo(Buffer*) {}
  public: virtual void Undo(Buffer*) = 0;

  DISALLOW_COPY_AND_ASSIGN(Record);
};

Record::Record() : m_pNext(nullptr), m_pPrev(nullptr) {
}

namespace internal {

//////////////////////////////////////////////////////////////////////
//
// TextRecord
//
class TextRecord : public Record {
  DECLARE_CASTABLE_CLASS(TextRecord, Record);

  protected: Posn m_lEnd;
  protected: Posn m_lStart;

  protected: TextRecord(Posn lStart, Posn lEnd);
  public: virtual ~TextRecord() = default;

  DISALLOW_COPY_AND_ASSIGN(TextRecord);
};

TextRecord::TextRecord(Posn lStart, Posn lEnd)
    : m_lEnd(lEnd),
      m_lStart(lStart) {
  DCHECK_LE(m_lStart, m_lEnd);
}

//////////////////////////////////////////////////////////////////////
//
// BeginRecord
//
class BeginRecord : public Record {
  DECLARE_CASTABLE_CLASS(BeginRecord, Record);

  private: const base::string16 name_;

  public: BeginRecord(const base::string16& name);
  public: virtual ~BeginRecord() = default;

  // Record
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(BeginRecord);
};

BeginRecord::BeginRecord(const base::string16& name) : name_(name) {
}

// Record
Posn BeginRecord::GetAfterRedo() const {
  return m_pNext->GetAfterRedo();
}

Posn BeginRecord::GetAfterUndo() const {
  return m_pNext->GetAfterUndo();
}

Posn BeginRecord::GetBeforeRedo() const {
  return m_pNext->GetBeforeRedo();
}

Posn BeginRecord::GetBeforeUndo() const {
  return m_pNext->GetBeforeUndo();
}

void BeginRecord::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->RecordEnd(name_);
}

//////////////////////////////////////////////////////////////////////
//
// DeleteRecord
//
class DeleteRecord : public TextRecord {
  DECLARE_CASTABLE_CLASS(DeleteRecord, TextRecord);

  private: class Chars {
    friend class DeleteRecord;

    private: Count m_cwch;
    private: Chars* m_pNext;

    public: void* operator new(size_t cb, UndoStack* pUndo, Posn lStart,
                               Posn lEnd) {
      return pUndo->Alloc(cb + sizeof(char16) * (lEnd - lStart));
    }

    public: Count GetLength() const { return m_cwch; }

    public: const char16* GetString() const {
      return reinterpret_cast<const char16*>(this + 1);
    }

    public: Chars(UndoStack* pUndo, Posn lStart, Posn lEnd)
        : m_cwch(lEnd - lStart),
          m_pNext(nullptr) {
      auto const pBuffer = pUndo->GetBuffer();
      auto pwch = const_cast<char16*>(GetString());
      for (Posn lPosn = lStart; lPosn < lEnd; lPosn++) {
        *pwch++ = pBuffer->GetCharAt(lPosn);
      }
    }
  };

  public: class EnumChars {
    private: Chars* m_pRunner;

    public: EnumChars(const DeleteRecord* p) : m_pRunner(p->m_pFirst) {
    }

    public: bool AtEnd() const { return nullptr == m_pRunner; }

    public: Chars* Get() const {
      DCHECK(!AtEnd());
      return m_pRunner;
    }

    public: void Next() {
      DCHECK(!AtEnd());
      m_pRunner = m_pRunner->m_pNext;
    }
  };

  private: Chars* m_pFirst;
  private: Chars* m_pLast;

  public: DeleteRecord(UndoStack* pUndo, Posn lStart, Posn lEnd);
  public: virtual ~DeleteRecord() = default;

  private: Chars* createChars(UndoStack* pUndo, Posn lStart, Posn lEnd);
  private: void insertChars(Buffer* pBuffer) const;
  // Merges new delete information into the last delete record if
  // o New delete doesn't start with newline.
  // o New delete doesn't end with newline.
  public: bool Merge(UndoStack* pUndo, Posn lStart, Posn lEnd);

  // Recrod
  private: virtual void Discard(UndoStack* pUndo) override;
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* pBuffer) override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(DeleteRecord);
};

DeleteRecord::DeleteRecord(UndoStack* pUndo, Posn lStart, Posn lEnd) :
    TextRecord(lStart, lEnd)
{
    m_pFirst = createChars(pUndo, lStart, lEnd);
    m_pLast = m_pFirst;
}

DeleteRecord::Chars* DeleteRecord::createChars(UndoStack* pUndo,
                                               Posn lStart, Posn lEnd) {
  return new(pUndo, lStart, lEnd) Chars(pUndo, lStart, lEnd);
}

void DeleteRecord::insertChars(Buffer* pBuffer) const {
  auto lPosn = m_lStart;
  foreach (EnumChars, oEnum, this) {
    auto const pChars = oEnum.Get();
    pBuffer->Insert(lPosn, pChars->GetString(), pChars->GetLength());
    lPosn += pChars->GetLength();
  }
}

bool DeleteRecord::Merge(UndoStack* pUndo, Posn lStart, Posn lEnd) {
  if (pUndo->GetBuffer()->GetCharAt(lStart) == 0x0A)
    return false;

  if (pUndo->GetBuffer()->GetCharAt(lEnd - 1) == 0x0A)
    return false;

  // For [Backspace] key
  // 1. abc|
  // 2. ab|
  if (m_lStart == lEnd) {
    auto const pChars = createChars(pUndo, lStart, lEnd);
    pChars->m_pNext = m_pFirst;
    m_pFirst = pChars;
    m_lStart = lStart;
    return true;
  }

  // For [Delete] key
  // 1. a|bc
  // 2. a|c
  if (m_lStart == lStart) {
    auto const pChars = createChars(pUndo, lStart, lEnd);
    m_pLast->m_pNext = pChars;
    m_pLast = pChars;
    m_lEnd += lEnd - lStart;
    return true;
  }

  return false;
}

// Record
void DeleteRecord::Discard(UndoStack* pUndo) {
  EnumChars oEnum(this);
  while (!oEnum.AtEnd()) {
      Chars* pChars = oEnum.Get();
      oEnum.Next();
      pUndo->Free(pChars);
  }
}

Posn DeleteRecord::GetAfterRedo() const {
  return m_lEnd;
}

Posn DeleteRecord::GetAfterUndo() const {
  return m_lEnd;
}

Posn DeleteRecord::GetBeforeRedo() const {
  return m_lStart;
}

Posn DeleteRecord::GetBeforeUndo() const {
  return m_lStart;
}

void DeleteRecord::Redo(Buffer* pBuffer) {
  insertChars(pBuffer);
  pBuffer->IncCharTick(1);
}

void DeleteRecord::Undo(Buffer* pBuffer) {
  insertChars(pBuffer);
  pBuffer->GetUndo()->RecordInsert(m_lStart, m_lEnd);
  pBuffer->IncCharTick(-1);
}

//////////////////////////////////////////////////////////////////////
//
// EndRecord
//
class EndRecord : public Record {
  DECLARE_CASTABLE_CLASS(EndRecord, Record);

  private: const base::string16 name_;

  public: EndRecord(const base::string16& name);
  public: virtual ~EndRecord() = default;

  public: bool CanMerge(const base::string16& name) const;

  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(EndRecord);
};

EndRecord::EndRecord(const base::string16& name) : name_(name) {
}

bool EndRecord::CanMerge(const base::string16& name) const {
  return name_ == name && '*' == name[0];
}

// Record
Posn EndRecord::GetAfterRedo() const {
  return m_pPrev->GetAfterRedo();
}

Posn EndRecord::GetAfterUndo() const {
  return m_pPrev->GetAfterUndo();
}

Posn EndRecord::GetBeforeRedo() const {
  return m_pPrev->GetBeforeRedo();
}

Posn EndRecord::GetBeforeUndo() const {
  return m_pPrev->GetBeforeUndo();
}

void EndRecord::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->RecordBegin(name_);
}

//////////////////////////////////////////////////////////////////////
//
// InsertRecord
//
class InsertRecord : public TextRecord {
  DECLARE_CASTABLE_CLASS(InsertRecord, TextRecord);

  public: InsertRecord(Posn lStart, Posn lEnd);
  public: virtual ~InsertRecord() = default;

  // Merge
  // Merge "Insert" record if
  // o [last][new]
  // o [new][last]
  // and there is no newline between last and new.
  public: bool Merge(Buffer* pBuffer, Posn lStart, Posn lEnd);

  // Record
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* pBuffer) override;;
  private: virtual void Undo(Buffer* pBuffer) override;;

  DISALLOW_COPY_AND_ASSIGN(InsertRecord);
};

InsertRecord::InsertRecord(Posn lStart, Posn lEnd)
    : TextRecord(lStart, lEnd) {
}

bool InsertRecord::Merge(Buffer* pBuffer, Posn lStart, Posn lEnd) {
  if (m_lEnd == lStart) {
    // [last][new]
    if (pBuffer->GetCharAt(m_lEnd - 1) != 0x0A) {
      m_lEnd = lEnd;
      return true;
    }
  } else if (m_lStart == lEnd) {
    // [new][last]
    if (pBuffer->GetCharAt(lEnd - 1) != 0x0A) {
      m_lStart = lStart;
      return true;
    }
  }
  return false;
}

// Record
Posn InsertRecord::GetAfterRedo() const {
  return m_lStart;
}

Posn InsertRecord::GetAfterUndo() const {
  return m_lStart;
}

Posn InsertRecord::GetBeforeRedo() const {
  return m_lEnd;
}

Posn InsertRecord::GetBeforeUndo() const {
  return m_lEnd;
}

void InsertRecord::Redo(Buffer* pBuffer) {
  pBuffer->Delete(m_lStart, m_lEnd);
  pBuffer->IncCharTick(1);
}

void InsertRecord::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->RecordDelete(m_lStart, m_lEnd);
  pBuffer->Delete(m_lStart, m_lEnd);
  pBuffer->IncCharTick(-1);
}

}  // namespace

using namespace internal;

//////////////////////////////////////////////////////////////////////
//
// UndoStack
//
UndoStack::UndoStack(Buffer* pBuffer)
    : m_eState(State_Log),
      m_hObjHeap(::HeapCreate(HEAP_NO_SERIALIZE, 0, 0)),
      m_pBuffer(pBuffer),
      m_pFirst(nullptr),
      m_pLast(nullptr),
      m_pRedo(nullptr),
      m_pUndo(nullptr) {
  m_pBuffer->AddObserver(this);
}

UndoStack::~UndoStack() {
}

void UndoStack::addRecord(Record* pRecord) {
  pRecord->m_pPrev = m_pLast;

  if (m_pLast)
    m_pLast->m_pNext = pRecord;

  m_pLast = pRecord;

  if (!m_pFirst)
    m_pFirst = pRecord;
}

void UndoStack::delRecord(Record* pRecord) {
  auto const pNext = pRecord->m_pNext;
  auto const pPrev = pRecord->m_pPrev;

  if (!pNext)
    m_pLast = pPrev;
  else
    pNext->m_pPrev = pPrev;

  if (!pPrev)
    m_pFirst = pNext;
  else
    pPrev->m_pNext = pNext;

  discardRecord(pRecord);
}

void UndoStack::discardRecord(Record* pRecord) {
  DCHECK_NE(pRecord, m_pUndo);
  DCHECK_NE(pRecord, m_pRedo);
  pRecord->Discard(this);
  ::HeapFree(m_hObjHeap, 0, pRecord);
}

void* UndoStack::Alloc(size_t cb) {
  return ::HeapAlloc(m_hObjHeap, 0, cb);
}

bool UndoStack::CanRedo() const {
  return m_pUndo != m_pRedo;
}

bool UndoStack::CanUndo() const {
  switch (m_eState) {
    case State_Log:
      return m_pLast;
    case State_Redo:
      return true;
    case State_Undo:
      return m_pUndo;
    default:
      return false;
  }
}

void UndoStack::CheckPoint() {
  switch (m_eState) {
    case State_Disabled:
      return;

    case State_Undo:
      break;

    case State_Redo:
      if (m_pRedo) {
        // Truncate executed redo logs
        // discarc m_pRedo->m_pNext ... m_pLast
        auto pRunner = m_pRedo->m_pNext;
        while (pRunner) {
          auto const pNext = pRunner->m_pNext;
          discardRecord(pRunner);
          pRunner = pNext;
        }
        m_pRedo->m_pNext = nullptr;
        m_pLast = m_pRedo;
      }
      break;
  }
  m_pRedo = nullptr;
  m_eState = State_Log;
}

void UndoStack::Empty() {
  if (m_hObjHeap) {
    ::HeapDestroy(m_hObjHeap);
    m_hObjHeap = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
  }

  m_eState = State_Log;
  m_pFirst = nullptr;
  m_pLast = nullptr;
  m_pRedo = nullptr;
  m_pUndo = nullptr;
}

void UndoStack::Free(void* pv) {
  ::HeapFree(m_hObjHeap, 0, pv);
}

void UndoStack::RecordBegin(const base::string16& name) {
  if (m_eState == State_Disabled)
    return;

  if (m_eState == State_Log && m_pLast &&
      m_pLast->is<EndRecord>()) {
    auto const pLast = m_pLast->as<EndRecord>();
    if (pLast->CanMerge(name)) {
      // If the last record is generated by same operation, we merge
      // the last record and new record.
      delRecord(m_pLast);
      return;
    }
  }

  BeginRecord* pRecord = new(this) BeginRecord(name);
  addRecord(pRecord);
}

void UndoStack::RecordDelete(Posn lStart, Posn lEnd) {
  auto const cwch = lEnd - lStart;
  if (cwch <= 0)
    return;

  if (State_Disabled == m_eState)
    return;

  if (m_pLast && m_pLast->is<DeleteRecord>()) {
    auto const pLast = m_pLast->as<DeleteRecord>();
    if (pLast->Merge(this, lStart, lEnd)) {
      m_pBuffer->IncCharTick(-1);
      return;
    }
  }

  auto const pRecord = new(this) DeleteRecord(this, lStart, lEnd);
  addRecord(pRecord);
}

void UndoStack::RecordEnd(const base::string16& pwszName) {
  if (m_eState == State_Disabled)
    return;

  if (m_pLast) {
    // Is empty block?
    if (m_pLast->is<BeginRecord>()) {
      delRecord(m_pLast);
      return;
    }
  }

  auto const pRecord = new(this) EndRecord(pwszName);
  addRecord(pRecord);
}

void UndoStack::RecordInsert(Posn lStart, Posn lEnd) {
  DCHECK_LE(lStart, lEnd);

  if (lStart >= lEnd || m_eState == State_Disabled)
    return;

  // Merge insert record.
  if (m_pLast && m_pLast->is<InsertRecord>()) {
    auto const pLast = m_pLast->as<InsertRecord>();
    if (pLast->Merge(m_pBuffer, lStart, lEnd)) {
      m_pBuffer->IncCharTick(-1);
      return;
    }
  }

  auto const pRecord = new(this) InsertRecord(lStart, lEnd);
  addRecord(pRecord);
}

Posn UndoStack::Redo(Posn lPosn, Count lCount) {
  if (!CanRedo())
    return -1;

  m_eState = State_Redo;

  int iDepth = 0;
  for (;;) {
    if (iDepth == 0) {
      if (lCount <= 0)
        break;
      --lCount;
    }

    if (!m_pRedo)
      break;

    auto const pRedo = m_pRedo;
    auto const pUndo = m_pUndo;

    if (pRedo == m_pUndo) {
      // We don't have no redo record.
      break;
    }

    // Are we are at redo point? If not, we don't execute.
    if (!lCount && !iDepth) {
      auto const lRedo = pRedo->GetBeforeRedo();
      if (lPosn != lRedo) {
        lPosn = lRedo;
        break;
      }
    }

    if (pRedo->is<BeginRecord>())
      ++iDepth;
    else if (pRedo->is<EndRecord>())
      --iDepth;

    pRedo->Redo(m_pBuffer);
    lPosn = pRedo->GetAfterRedo();
    m_pRedo = pRedo->m_pPrev;
    m_pUndo = pUndo != nullptr ? pUndo->m_pNext : m_pFirst;
  }
  return lPosn;
}

Posn UndoStack::Undo(Posn lPosn, Count lCount) {
  if (!CanUndo())
    return -1;

  if (m_eState == State_Redo) {
    // Truncate edit log betwee redo.next to last.
    CheckPoint();
  }

  if (m_eState != State_Undo)
    m_pUndo = m_pLast;

  m_eState = State_Undo;

  int iDepth = 0;
  for (;;) {
    if (!iDepth) {
      if (lCount <= 0)
        break;
      --lCount;
    }

    auto const pRecord = m_pUndo;
    if (!pRecord) {
      DCHECK(!iDepth);
      break;
    }

    if (!lCount && !iDepth) {
      auto const lUndo = pRecord->GetBeforeUndo();
      if (lPosn != lUndo) {
        lPosn = lUndo;
        break;
      }
    }

    if (pRecord->is<BeginRecord>())
      --iDepth;
    else if (pRecord->is<EndRecord>())
      ++iDepth;

    pRecord->Undo(m_pBuffer);
    lPosn = pRecord->GetAfterUndo();
    m_pUndo = pRecord->m_pPrev;
  }

  m_pRedo = m_pLast;
  return lPosn;
}


// BufferMutationObserver
void UndoStack::DidInsertAt(Posn offset, size_t length) {
  if (m_eState != State_Log)
    return;
  CheckPoint();
  RecordInsert(offset, static_cast<Posn>(offset + length));
}

void UndoStack::WillDeleteAt(Posn offset, size_t length) {
  if (m_eState != State_Log)
    return;
  CheckPoint();
  RecordDelete(offset, static_cast<Posn>(offset + length));
}

//////////////////////////////////////////////////////////////////////
//
// UndoBlock
//
UndoBlock::~UndoBlock() {
  buffer_->EndUndoGroup(name_);
}

UndoBlock::UndoBlock(Buffer* buffer, const base::string16& name)
    : buffer_(buffer), name_(name) {
  buffer->StartUndoGroup(name);
}

void Buffer::EndUndoGroup(const base::string16& name) {
  undo_stack_->CheckPoint();
  undo_stack_->RecordEnd(name);
}

void Buffer::StartUndoGroup(const base::string16& name) {
  undo_stack_->CheckPoint();
  undo_stack_->RecordBegin(name);
}

}  // namespace text
