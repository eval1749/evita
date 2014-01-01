//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit range
// listener/winapp/ed_range.h
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Range.h#5 $
//
#if !defined(INCLUDE_edit_range_h)
#define INCLUDE_edit_range_h

namespace Edit
{

/// <summary>
///  Represents a range in buffer.
/// </summary>
class Range //: public ObjectInHeap
{
    friend class Buffer;

    public: struct Information
    {
        bool    m_fLineNum;
        bool    m_fColumn;
        Count   m_lLineNum;
        Count   m_lColumn;
    }; // Information

    protected: enum Kind
    {
        Kind_Range,
        Kind_Selection,
    }; // Kind

    public: void* operator new(size_t cb, HANDLE h)
        { return ::HeapAlloc(h, 0, cb); }

    private: void operator delete(void*) {}

    public: void destroy();

    private: Kind       m_eKind;
    private: Posn       m_lStart;
    private: Posn       m_lEnd;
    private: Range*     m_pNext;
    private: Range*     m_pPrev;
    private: Buffer*    m_pBuffer;

    public: Range(
            Buffer* pBuffer,
            Kind    eKind   = Kind_Range,
            Posn    lStart  = 0,
            Posn    lEnd    = 0) :
        m_eKind(eKind),
        m_lStart(lStart),
        m_lEnd(lEnd),
        m_pBuffer(pBuffer),
        m_pNext(NULL),
        m_pPrev(NULL) {}

    public: Range(Buffer*, Posn, Posn);
    public: Range(Range*);

    public: ~Range();

    // [A]
    public: StringCase AnalyzeCase() const;

    // [C]
    public: void  Collapse(CollapseWhich = Collapse_Start);
    public: void  Capitalize();
    public: void  CapitalizeAll();
    public: Count Copy();
    public: Count Cut();

    // [D]
    public: Count Delete(Unit eUnit = Unit_Char, Count = 1);
    public: void  Downcase();

    // [E]
    public:  Count EndOf(Unit, bool = false);
    private: Posn  ensurePosn(Posn) const;

    // [F]
    public: Posn FindCloseParen(bool = false);
    public: Posn FindFirstChar(char16) const;
    //public: Posn FindLastChar(char16) const;
    public: Posn FindOpenParen(bool = false);

    // [G]
    public: Buffer* GetBuffer() const { return m_pBuffer; }
    public: Posn    GetEnd()    const { return m_lEnd; }
    public: void    GetInformation(Information*, Count = Count_Max) const;
    public: Posn    GetStart()  const { return m_lStart; }
    public: char16* GetText() const;
    public: char16* GetText(StringResult*) const;

    // [I]
    public: void Indent();

    // [M]
    public: Count Move(Unit, Count);
    public: Count MoveEnd(Unit, Count);
    public: Count MoveEndWhile(const char16*, Count = Count_Forward);
    public: Count MoveEndWhile(uint, Count = Count_Forward);
    public: Count MoveStart(Unit, Count);
    public: Count MoveStartWhile(const char16*, Count = Count_Forward);
    public: Count MoveStartWhile(uint, Count = Count_Forward);
    public: Count MoveWhile(const char16*, Count = Count_Forward);

    // [O]
    public: void Outdent();

    // [P]
    public: void Paste();

    // [S]
    public: Posn  SetEnd(Posn);
    public: void  SetRange(Posn, Posn);
    public: void  SetRange(Range*);
    public: Posn  SetStart(Posn);
    public: Count SetText(const char16*);
    public: Count SetText(const char16*, int);
    public: Count StartOf(Unit, bool = false);

    // [U]
    public:  void  Upcase();
    private: void  updateAnchor(Posn, Posn);
    private: void  updateGoal();
}; // Range

} // Edit

#endif //!defined(INCLUDE_edit_range_h)
