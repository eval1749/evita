//////////////////////////////////////////////////////////////////////////////
//
// evcl - editor - undo manager
// listener/winapp/ed_undo.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Undo.cpp#2 $
//
#define DEBUG_UNDO 0
#include "evita/text/undo_manager.h"

#include "evita/text/buffer.h"

namespace text
{


#if DEBUG_UNDO
static const char16* const
k_rgpwszRecord[] =
{
    L"Begin",
    L"Delete",
    L"End",
    L"Insert",
}; // k_rgpwszRecord
#endif // DEBUG_UNDO

//////////////////////////////////////////////////////////////////////
//
// Record
//
class Record
{
    friend class UndoManager;

    public: void* operator new(size_t cb, UndoManager* p)
        { return p->Alloc(cb); }

    private: void operator delete(void*) {}

    public: enum Kind
    {
        Kind_Begin,
        Kind_Delete,
        Kind_End,
        Kind_Insert,
    }; // Kind

    protected: Record* m_pNext;
    protected: Record* m_pPrev;

    public: Record() : m_pNext(NULL), m_pPrev(NULL) {}

    public: virtual void   Discard(UndoManager*) {}
    public: virtual Posn   GetAfterRedo() const = 0;
    public: virtual Posn   GetAfterUndo() const = 0;
    public: virtual Posn   GetBeforeRedo() const = 0;
    public: virtual Posn   GetBeforeUndo() const = 0;
    public: virtual Kind   GetKind() const = 0;
    public: virtual size_t GetSize() const = 0;
    public: virtual void   Redo(Buffer*) {}
    public: virtual void   Undo(Buffer*) = 0;

    public: template<class T> bool Is() const
        { return GetKind() == T::GetKind_(); }

    public: template<class T> T* StaticCast() { 
      ASSERT(Is<T>());
      // warning C4946: reinterpret_cast used between related classes: 
      // 'class1' and 'class2'
      #pragma warning(suppress: 4946)
      return reinterpret_cast<T*>(this); 
    }

    #if DEBUG_UNDO
    public: const char16* GetSymbol() const
        { return k_rgpwszRecord[GetKind()]; }
    #endif // endif DEBUG_UNDO
}; // Record

namespace Internal
{

//////////////////////////////////////////////////////////////////////
//
// TextRecord
//
class TextRecord : public Record
{
    protected: Posn m_lEnd;
    protected: Posn m_lStart;

    protected: TextRecord(Posn lStart, Posn lEnd) :
        m_lEnd(lEnd),
        m_lStart(lStart)
    {
        ASSERT(m_lStart <= m_lEnd);
    } // TextRecord
}; // TextRecord


//////////////////////////////////////////////////////////////////////
//
// BeginRecord
//
class BeginRecord : public Record
{
    private: const base::string16  name_;

    // ctor
    public: BeginRecord(const base::string16& name) : name_(name) {}
    public: virtual ~BeginRecord() = default;

    // [G]
    public: static Kind GetKind_() { return Kind_Begin; }

    public: virtual Posn GetAfterRedo() const
        { return m_pNext->GetAfterRedo(); }

    public: virtual Posn GetAfterUndo() const
        { return m_pNext->GetAfterUndo(); }

    public: virtual Posn GetBeforeRedo() const
        { return m_pNext->GetBeforeRedo(); }

    public: virtual Posn GetBeforeUndo() const
        { return m_pNext->GetBeforeUndo(); }

    public: virtual Kind   GetKind() const { return GetKind_(); }
    public: virtual size_t GetSize() const { return sizeof(BeginRecord); }

    // [U]
    public: virtual void Undo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p\n", this);
        }
        #endif // DEBUG_UNDO
        pBuffer->GetUndo()->RecordEnd(name_);
    } // Undo

    DISALLOW_COPY_AND_ASSIGN(BeginRecord);
}; // BeginRecord


//////////////////////////////////////////////////////////////////////
//
// DeleteRecord
//
class DeleteRecord : public TextRecord
{
    private: class Chars
    {
        friend class DeleteRecord;

        private: Count  m_cwch;
        private: Chars* m_pNext;

        public: void* operator new(
            size_t          cb,
            UndoManager*    pUndo,
            Posn            lStart,
            Posn            lEnd )
        {
            return pUndo->Alloc(cb + sizeof(char16) * (lEnd - lStart));
        } // new

        public: Count GetLength() const
            { return m_cwch; }

        public: const char16* GetString() const
            { return reinterpret_cast<const char16*>(this + 1); }

        public: Chars(UndoManager* pUndo, Posn lStart, Posn lEnd) :
            m_cwch(lEnd - lStart),
            m_pNext(NULL)
        {
            Buffer* pBuffer = pUndo->GetBuffer();
            char16* pwch = const_cast<char16*>(GetString());
            for (Posn lPosn = lStart; lPosn < lEnd; lPosn++)
            {
                *pwch++ = pBuffer->GetCharAt(lPosn);
            } // for each posn
        } // Chars
    }; // Chars

    private: Chars* m_pFirst;
    private: Chars* m_pLast;

    // ctor
    public: DeleteRecord(UndoManager* pUndo, Posn lStart, Posn lEnd) :
        TextRecord(lStart, lEnd)
    {
        m_pFirst = createChars(pUndo, lStart, lEnd);
        m_pLast  = m_pFirst;
    } // DeleteRecord

    // [C]
    private: Chars* createChars(UndoManager* pUndo, Posn lStart, Posn lEnd)
    {
        return new(pUndo, lStart, lEnd) Chars(pUndo, lStart, lEnd);
    } // createChars

    // [D]
    public: virtual void Discard(UndoManager* pUndo)
    {
        EnumChars oEnum(this);
        while (! oEnum.AtEnd())
        {
            Chars* pChars = oEnum.Get();
            oEnum.Next();
            pUndo->Free(pChars);
        } // for each char
    } // Discard

    // [E]
    public: class EnumChars
    {
        private: Chars* m_pRunner;

        public: EnumChars(const DeleteRecord* p) :
            m_pRunner(p->m_pFirst) {}

        public: bool AtEnd() const { return NULL == m_pRunner; }

        public: Chars* Get() const
            { ASSERT(! AtEnd()); return m_pRunner; }

        public: void Next()
            { ASSERT(! AtEnd()); m_pRunner = m_pRunner->m_pNext; }
    }; // EnumChars

    // [G]
    public: virtual Posn GetAfterRedo()  const { return m_lEnd; }
    public: virtual Posn GetAfterUndo()  const { return m_lEnd; }
    public: virtual Posn GetBeforeRedo() const { return m_lStart; }
    public: virtual Posn GetBeforeUndo() const { return m_lStart; }

    public: static Kind  GetKind_() { return Kind_Delete; }
    public: virtual Kind GetKind() const { return GetKind_(); }

    public: virtual size_t GetSize() const
    {
        size_t cb = sizeof(DeleteRecord);
        cb += sizeof(char16) * (m_lEnd - m_lStart);
        return cb;
    } // GetSize

    // [I]
    // insertChars
    private: void insertChars(Buffer* pBuffer) const
    {
        Posn lPosn = m_lStart;
        foreach (EnumChars, oEnum, this)
        {
            Chars* pChars = oEnum.Get();

            pBuffer->InternalInsert(
                lPosn,
                pChars->GetString(),
                pChars->GetLength() );

            lPosn += pChars->GetLength();
        } // lPosn
    } // insertChars

    // [M]
    // Merge
    //  Merges new delete information into the last delete record if
    //   o New delete doesn't start with newline.
    //   o New delete doesn't end with newline.
    public: bool Merge(UndoManager* pUndo, Posn lStart, Posn lEnd)
    {
        if (pUndo->GetBuffer()->GetCharAt(lStart) == 0x0A)
        {
            return false;
        }

        if (pUndo->GetBuffer()->GetCharAt(lEnd - 1) == 0x0A)
        {
            return false;
        }

        // For [Backspace] key
        // 1. abc|
        // 2. ab|
        if (m_lStart == lEnd)
        {
            #if DEBUG_UNDO
            {
                DEBUG_PRINTF("%p delete[%d,%d] extend start to %d\n",
                    this, m_lStart, m_lEnd, lStart);
            }
            #endif // DEBUG_UNDO
            Chars* pChars  = createChars(pUndo, lStart, lEnd);
            pChars->m_pNext = m_pFirst;
            m_pFirst = pChars;
            m_lStart = lStart;
            return true;
        } // if

        // For [Delete] key
        // 1. a|bc
        // 2. a|c
        if (m_lStart == lStart)

        {
            #if DEBUG_UNDO
            {
                DEBUG_PRINTF("%p delete[%d,%d] extend end to %d\n",
                    this, m_lStart, m_lEnd, m_lEnd + (lEnd - lStart) );
            }
            #endif // DEBUG_UNDO
            Chars* pChars = createChars(pUndo, lStart, lEnd);
            m_pLast->m_pNext = pChars;
            m_pLast = pChars;
            m_lEnd += lEnd - lStart;
            return true;
        }

        return false;
    } // Merge

    // [R]
    public: virtual void Redo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p delete [%d,%d]\n", this, m_lStart, m_lEnd);
        }
        #endif // DEBUG_UNDO
        insertChars(pBuffer);
        pBuffer->IncCharTick(1);
    } // Redo

    // [U]
    public: virtual void Undo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p delete [%d,%d]\n", this, m_lStart, m_lEnd);
        }
        #endif // DEBUG_UNDO
        insertChars(pBuffer);
        pBuffer->GetUndo()->RecordInsert(m_lStart, m_lEnd);
        pBuffer->IncCharTick(-1);
    } // Undo
}; // DeleteRecord


//////////////////////////////////////////////////////////////////////
//
// EndRecord
//
class EndRecord : public Record
{
    private: const base::string16 name_;

    // ctor
    public: EndRecord(const base::string16& name) : name_(name) {}
    public: virtual ~EndRecord() = default;

    // [I]
    public: bool CanMerge(const base::string16& name)
    {
        return name_ == name && '*' == name[0];
    }

    // [G]
    public: static Kind GetKind_() { return Kind_End; }

    public: virtual Posn GetAfterRedo() const
        { return m_pPrev->GetAfterRedo(); }

    public: virtual Posn GetAfterUndo() const
        { return m_pPrev->GetAfterUndo(); }

    public: virtual Posn GetBeforeRedo() const
        { return m_pPrev->GetBeforeRedo(); }

    public: virtual Posn GetBeforeUndo() const
        { return m_pPrev->GetBeforeUndo(); }

    public: virtual Kind   GetKind() const { return GetKind_(); }
    public: virtual size_t GetSize() const { return sizeof(EndRecord); }

    // [U]
    public: virtual void Undo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p\n", this);
        }
        #endif // DEBUG_UNDO
        pBuffer->GetUndo()->RecordBegin(name_);
    } // Undo

    DISALLOW_COPY_AND_ASSIGN(EndRecord);
}; // EndRecord


//////////////////////////////////////////////////////////////////////
//
// InsertRecord
//
class InsertRecord : public TextRecord
{
    public: virtual Posn GetAfterRedo()  const { return m_lStart; }
    public: virtual Posn GetAfterUndo()  const { return m_lStart; }
    public: virtual Posn GetBeforeRedo() const { return m_lEnd; }
    public: virtual Posn GetBeforeUndo() const { return m_lEnd; }

    public: static Kind GetKind_()         { return Kind_Insert; }
    public: virtual Kind GetKind()   const { return GetKind_(); }
    public: virtual size_t GetSize() const { return sizeof(InsertRecord); }

    public: virtual void Redo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p insert [%d,%d]\n", this, m_lStart, m_lEnd);
        }
        #endif // DEBUG_UNDO
        pBuffer->InternalDelete(m_lStart, m_lEnd);
        pBuffer->IncCharTick(1);
    } // Redo

    public: virtual void Undo(Buffer* pBuffer)
    {
        #if DEBUG_UNDO
        {
            DEBUG_PRINTF("%p insert [%d,%d]\n", this, m_lStart, m_lEnd);
        }
        #endif // DEBUG_UNDO
        pBuffer->GetUndo()->RecordDelete(m_lStart, m_lEnd);
        pBuffer->InternalDelete(m_lStart, m_lEnd);
        pBuffer->IncCharTick(-1);
    } // Undo

    public: InsertRecord(Posn lStart, Posn lEnd) :
        TextRecord(lStart, lEnd) {}

    // Merge
    //  Merge "Insert" record if
    //   o [last][new]
    //   o [new][last]
    //  and there is no newline between last and new.
    public: bool Merge(Buffer* pBuffer, Posn lStart, Posn lEnd)
    {
        if (m_lEnd == lStart)
        {
            // [last][new]
            if (pBuffer->GetCharAt(m_lEnd - 1) != 0x0A)
            {
                m_lEnd = lEnd;
                return true;
            }
        }
        else if (m_lStart == lEnd)
        {
            // [new][last]
            if (pBuffer->GetCharAt(lEnd - 1) != 0x0A)
            {
                m_lStart = lStart;
                return true;
            }
        }
        return false;
    } // Merge
}; // InsertRecord

} // Internal

using namespace Internal;

//////////////////////////////////////////////////////////////////////
//
// UndoManager ctor
//
UndoManager::UndoManager(Buffer* pBuffer) :
    m_cb(0),
    m_eState(State_Log),
    m_fMerge(true),
    m_fTruncate(false),
    m_hObjHeap(NULL),
    m_pBuffer(pBuffer),
    m_pFirst(NULL),
    m_pLast(NULL),
    m_pRedo(NULL),
    m_pUndo(NULL)
{
    m_hObjHeap = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);

    #if DEBUG_UNDO
      DEBUG_PRINTF("%p: new heap=%p\n", this, m_hObjHeap);
    #endif
} // UndoManager::UndoManager

UndoManager::~UndoManager() {
}

//////////////////////////////////////////////////////////////////////
//
// UndoManager::addRecord
//
void UndoManager::addRecord(Record* pRecord)
{
    pRecord->m_pPrev = m_pLast;

    if (NULL != m_pLast)
    {
        m_pLast->m_pNext = pRecord;
    } // if

    m_pLast = pRecord;

    if (m_pFirst == NULL)
    {
        m_pFirst = pRecord;
    }
} // UndoManager::addRecord


//////////////////////////////////////////////////////////////////////
//
// UndoManager::delRecord
//
void UndoManager::delRecord(Record* pRecord)
{
    Record* pNext = pRecord->m_pNext;
    Record* pPrev = pRecord->m_pPrev;

    if (pNext == NULL)
    {
        m_pLast = pPrev;
    }
    else
    {
        pNext->m_pPrev = pPrev;
    } // if

    if (pPrev == NULL)
    {
        m_pFirst = pNext;
    }
    else
    {
        pPrev->m_pNext = pNext;
    } // if

    discardRecord(pRecord);
} // UndoManager::delRecord


//////////////////////////////////////////////////////////////////////
//
// UndoManager::discardRecord
//
void UndoManager::discardRecord(Record* pRecord)
{
    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("discard %p %s\n",
            pRecord, pRecord->GetSymbol() );
    }
    #endif // DEBUG_UNDO

    ASSERT(pRecord != m_pUndo);
    ASSERT(pRecord != m_pRedo);

    m_cb -= pRecord->GetSize();
    pRecord->Discard(this);
    ::HeapFree(m_hObjHeap, 0, pRecord);
} // UndoManager::discardRecord


//////////////////////////////////////////////////////////////////////
//
// UndoManager::Alloc
//
void* UndoManager::Alloc(size_t cb)
{
    m_cb += cb;
    return ::HeapAlloc(m_hObjHeap, 0, cb);
} // UndoManager::Alloc


//////////////////////////////////////////////////////////////////////
//
// UndoManager::CanRedo
//
bool UndoManager::CanRedo() const
{
    return m_pUndo != m_pRedo;
} // UndoManager::CanRedo


//////////////////////////////////////////////////////////////////////
//
// UndoManager::CanUndo
//
bool UndoManager::CanUndo() const
{
    switch (m_eState)
    {
    case State_Log:
        return NULL != m_pLast;

    case State_Redo:
        return true;

    case State_Undo:
        return NULL != m_pUndo;

    default:
        return false;
    } // switch state
} // UndoManager::CanUndo


//////////////////////////////////////////////////////////////////////
//
// UndoManager::CheckPoint
//
void UndoManager::CheckPoint()
{
    switch (m_eState)
    {
    case State_Disabled:
        return;

    case State_Undo:
        if (m_fTruncate) 
        {
            TruncateLog();
        }
        break;

    case State_Redo:
        if (m_fTruncate)
        {
            TruncateLog();
        }
        else
        {
            // Truncate executed redo logs
            //  discarc m_pRedo->m_pNext ... m_pLast
            if (NULL != m_pRedo)
            {
                Record* pRunner = m_pRedo->m_pNext;
                while (NULL != pRunner)
                {
                    Record* pNext = pRunner->m_pNext;
                    discardRecord(pRunner);
                    pRunner = pNext;
                } // while
                m_pRedo->m_pNext = NULL;
                m_pLast = m_pRedo;
            } // if
        } // if
        break;
    } // switch state
    m_pRedo = NULL;
    m_eState = State_Log;
} // UndoManager::CheckPoint


//////////////////////////////////////////////////////////////////////
//
// UndoManager::Empty
//
void UndoManager::Empty()
{
    #if DEBUG_UNDO
      DEBUG_PRINTF("%p: heap=%p\n", this, m_hObjHeap);
    #endif

    if (NULL != m_hObjHeap)
    {
        ::HeapDestroy(m_hObjHeap);
        m_hObjHeap  = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
    }

    m_eState = State_Log;

    m_pFirst = NULL;
    m_pLast  = NULL;
    m_pRedo  = NULL;
    m_pUndo  = NULL;

    #if DEBUG_UNDO
      DEBUG_PRINTF("%p: new heap=%p\n", this, m_hObjHeap);
    #endif
} // UndoManager::Empty


//////////////////////////////////////////////////////////////////////
//
// UndoManager::Free
//
void UndoManager::Free(void* pv)
{
    ::HeapFree(m_hObjHeap, 0, pv);
} // UndoManager::Free


//////////////////////////////////////////////////////////////////////
//
// UndoManager::RecordBegin
//
void UndoManager::RecordBegin(const base::string16& name)
{
    if (m_eState == State_Disabled) return;

    if (m_fMerge && 
        m_eState == State_Log &&
        NULL != m_pLast && 
        m_pLast->Is<EndRecord>() )
    {
        EndRecord* pLast = m_pLast->StaticCast<EndRecord>();
        if (pLast->CanMerge(name))
        {
            //  If the last record is generated by same operation, we merge
            //  the last record and new record.
            delRecord(m_pLast);
            return;
        } // if
    } // if

    BeginRecord* pRecord = new(this) BeginRecord(name);
    addRecord(pRecord);

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("%p %s\n", pRecord, pRecord->GetSymbol());
    }
    #endif // DEBUG_UNDO
} // UndoManager::RecordBegin


//////////////////////////////////////////////////////////////////////
//
// UndoManager::RecordDelete
//
void UndoManager::RecordDelete(Posn lStart, Posn lEnd)
{
    Count cwch = lEnd - lStart;
    if (cwch <= 0)
    {
        return;
    }

    if (State_Disabled == m_eState)
    {
        return;
    }

    if (m_fMerge && NULL != m_pLast && m_pLast->Is<DeleteRecord>())
    {
        DeleteRecord* pLast = m_pLast->StaticCast<DeleteRecord>();
        if (pLast->Merge(this, lStart, lEnd))
        {
            m_pBuffer->IncCharTick(-1);
            return;
        } // if
    } // if

    DeleteRecord* pRecord = new(this) DeleteRecord(this, lStart, lEnd);

    addRecord(pRecord);

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("%p delete[%d,%d]\n", pRecord, lStart, lEnd );
    }
    #endif // DEBUG_UNDO
} // UndoManager::RecordDelete


//////////////////////////////////////////////////////////////////////
//
// UndoManager::RecordEnd
//
void UndoManager::RecordEnd(const base::string16& pwszName)
{
    if (m_eState == State_Disabled) return;

    if (NULL != m_pLast)
    {
        // Is empty block?
        if (m_pLast->Is<BeginRecord>())
        {
            #if DEBUG_UNDO
            {
                DEBUG_PRINTF("Ignore empty undo record.\n");
            }
            #endif // DEBUG_UNDO
            delRecord(m_pLast);
            return;
        } // if

#if 0
        // Is single unit block?
        Record* pPrev = m_pLast->m_pPrev;
        if (pPrev != NULL && pPrev->Is<BeginRecord>())
        {
            #if DEBUG_UNDO
            {
                DEBUG_PRINTF("Single unit block.\n");
            }
            #endif // DEBUG_UNDO
            delRecord(pPrev);
            return;
        } // if
#endif
    } // if

    EndRecord* pRecord = new(this) EndRecord(pwszName);
    addRecord(pRecord);

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("%p %s\n", pRecord, pRecord->GetSymbol());
    }
    #endif // DEBUG_UNDO
} // UndoManager::RecordEnd


//////////////////////////////////////////////////////////////////////
//
// UndoManager::RecordInsert
//
void UndoManager::RecordInsert(Posn lStart, Posn lEnd)
{
    ASSERT(lStart <= lEnd);

    if (lStart >= lEnd) return;
    if (m_eState == State_Disabled) return;

    // Merge insert record.
    if (m_fMerge && NULL != m_pLast && m_pLast->Is<InsertRecord>())
    {
        InsertRecord* pLast = m_pLast->StaticCast<InsertRecord>();
        if (pLast->Merge(m_pBuffer, lStart, lEnd))
        {
            #if DEBUG_UNDO
            {
                DEBUG_PRINTF("merge [%d,%d]\n", lStart, lEnd);
            }
            #endif // DEBUG_UNDO
            m_pBuffer->IncCharTick(-1);
            return;
        }
    } // if

    InsertRecord* pRecord = new(this) InsertRecord(lStart, lEnd);
    addRecord(pRecord);

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("%p %s[%d,%d]\n",
            pRecord, pRecord->GetSymbol(), lStart, lEnd );
    }
    #endif // DEBUG_UNDO
} // UndoManager::RecordInsert


//////////////////////////////////////////////////////////////////////
//
// UndoManager::Redo
//
Posn UndoManager::Redo(Posn lPosn, Count lCount)
{
    if (! CanRedo()) return -1;

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("state=%d cur=%d\n", m_eState, lPosn);
    }
    #endif // DEBUG_UNDO

    m_eState = State_Redo;

    int iDepth = 0;
    for (;;)
    {
        if (iDepth == 0)
        {
            if (lCount <= 0) break;
            lCount -= 1;
        } // if

        if (NULL == m_pRedo) break;

        Record* pRedo = m_pRedo;
        Record* pUndo = m_pUndo;

        if (pRedo == m_pUndo)
        {
            // We don't have no redo record.
            break;
        }

        // Are we are at redo point? If not, we don't execute.
        if (lCount == 0 && iDepth == 0)
        {
            Posn lRedo = pRedo->GetBeforeRedo();
            if (lPosn != lRedo)
            {
                lPosn = lRedo;
                break;
            }
        } // if

        switch (pRedo->GetKind())
        {
        case Record::Kind_Begin:
            iDepth += 1;
            break;
        case Record::Kind_End:
            iDepth -= 1;
            break;
        } // switch kind

        pRedo->Redo(m_pBuffer);

        lPosn = pRedo->GetAfterRedo();

        m_pRedo = pRedo->m_pPrev;
        m_pUndo = pUndo != NULL ? pUndo->m_pNext : m_pFirst;
    } // for

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("state=%d new=%d\n", m_eState, lPosn);
    }
    #endif // DEBUG_UNDO

    return lPosn;
} // UndoManager::Redo


// UndoManager::TruncateLog
//  Discards records from m_pUndo (exclusive) to end of log.
void UndoManager::TruncateLog()
{
    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("s=%d u=%p p=%p\n", m_eState, m_pUndo, m_pRedo);
    }
    #endif // DEBUG_UNDO

    Record* pRunner;
    if (NULL == m_pUndo)
    {
        pRunner = m_pFirst;
        m_pFirst = NULL;
    }
    else
    {
        pRunner = m_pUndo->m_pNext;
        m_pUndo->m_pNext = NULL;
    } // if

    m_pRedo = NULL;

    while (pRunner != m_pUndo)
    {
        if (NULL == pRunner) break;
        Record* pNext = pRunner->m_pNext;
        discardRecord(pRunner);
        pRunner = pNext;
    } // while

    m_pLast = m_pUndo;
} // UndoManager::TruncateLog


// UndoManager::Undo
Posn UndoManager::Undo(Posn lPosn, Count lCount)
{
    if (! CanUndo()) return -1;

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("state=%d cur=%d\n", m_eState, lPosn);
    }
    #endif // DEBUG_UNDO


    if (m_eState == State_Redo)
    {
        // Truncate edit log betwee redo.next to last.
        CheckPoint();
    }

    if (m_eState != State_Undo)
    {
        m_pUndo = m_pLast;
    }

    m_eState = State_Undo;

    int iDepth = 0;
    for (;;)
    {
        if (iDepth == 0)
        {
            if (lCount <= 0) break;
            lCount -= 1;
        } // if


        Record* pRecord = m_pUndo;
        if (pRecord == NULL)
        {
            ASSERT(iDepth == 0);
            break;
        } // if

        if (lCount == 0 && iDepth == 0)
        {
            Posn lUndo = pRecord->GetBeforeUndo();
            if (lPosn != lUndo)
            {
                lPosn = lUndo;
                break;
            }
        } // if

        switch (pRecord->GetKind())
        {
        case Record::Kind_Begin:
            iDepth -= 1;
            break;

        case Record::Kind_End:
            iDepth += 1;
            break;
        } // switch kind

        pRecord->Undo(m_pBuffer);

        lPosn = pRecord->GetAfterUndo();

        m_pUndo = pRecord->m_pPrev;
    } // for

    #if DEBUG_UNDO
    {
        DEBUG_PRINTF("state=%d new=%d\n", m_eState, lPosn);
    }
    #endif // DEBUG_UNDO

    m_pRedo = m_pLast;

    return lPosn;
} // UndoManager::Undo

UndoBlock::~UndoBlock() {
  buffer_->EndUndoGroup(name_);
}

UndoBlock::UndoBlock(Buffer* buffer, const base::string16& name)
    : buffer_(buffer), name_(name) {
  buffer->StartUndoGroup(name);
}

void Buffer::EndUndoGroup(const base::string16& name) {
  undo_manager_->CheckPoint();
  undo_manager_->RecordEnd(name);
}

void Buffer::StartUndoGroup(const base::string16& name) {
  undo_manager_->CheckPoint();
  undo_manager_->RecordBegin(name);
}
}  // namespace text
