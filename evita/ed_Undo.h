//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Undo.h#1 $
//
#if !defined(INCLUDE_listener_winapp_editor_undo_h)
#define INCLUDE_listener_winapp_editor_undo_h

namespace Edit
{

class Record;

//////////////////////////////////////////////////////////////////////
//
// UndoManager
//
//  m_cb
//    Holds hint of total size of edit log records.
//  m_fMerge
//    We merge consecutive insert/delete edit log records.
//  m_fTruncate
//    We truncate edit log if undo/redo operation is interrupted.
//
class UndoManager : public ObjectInHeap
{
    public: enum State
    {
        State_Disabled,
        State_Log,
        State_Redo,
        State_Undo,
    }; // State

    private: size_t  m_cb;
    private: State   m_eState;
    private: bool    m_fMerge;
    private: bool    m_fTruncate;
    private: HANDLE  m_hObjHeap;
    private: Buffer* m_pBuffer;
    private: Record* m_pFirst;
    private: Record* m_pLast;
    private: Record* m_pRedo;
    private: Record* m_pUndo;

    // ctor
    public: UndoManager(Buffer*);

    // [A]
    public: void*   Alloc(size_t);

    // [C]
    public: bool    CanRedo() const;
    public: bool    CanUndo() const;
    public: void    CheckPoint();

    // [E]
    public: void    Empty();

    // [F]
    public: void    Free(void*);

    // [G]
    public: Buffer* GetBuffer() const { return m_pBuffer; }
    public: size_t  GetSize()   const { return m_cb; }

    // [R]
    public: void    RecordBegin(const char16*);
    public: void    RecordDelete(Posn, Posn);
    public: void    RecordEnd(const char16*);
    public: void    RecordInsert(Posn, Posn);
    public: Posn    Redo(Posn, Count);

    // [T]
    public: void    TruncateLog();

    // [U]
    public: Posn    Undo(Posn, Count);

    // Private methods
    private: void addRecord(Record*);
    private: void delRecord(Record*);
    private: void discardRecord(Record*);
}; // UndoManager

} // Edit

#endif //!defined(INCLUDE_listener_winapp_editor_undo_h)
