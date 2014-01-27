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

#include "base/strings/string16.h"

namespace text
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
    private: Buffer*    m_pBuffer;

    public: Range(
            Buffer* pBuffer,
            Kind    eKind   = Kind_Range,
            Posn    lStart  = 0,
            Posn    lEnd    = 0) :
        m_eKind(eKind),
        m_lStart(lStart),
        m_lEnd(lEnd),
        m_pBuffer(pBuffer) {
    }

    public: Range(Buffer*, Posn, Posn);
    public: explicit Range(const Range& other);

    public: ~Range();

    // [A]
    public: StringCase AnalyzeCase() const;

    // [C]
    public: void  Collapse(CollapseWhich = Collapse_Start);
    public: void  Capitalize();
    public: void  CapitalizeAll();
    public: Count Copy();

    // [D]
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
    public: base::string16 GetText() const;

    // [M]
    public: Count Move(Unit, Count);
    public: Count MoveEnd(Unit, Count);
    public: Count MoveStart(Unit, Count);

    // [P]
    public: void Paste();

    // [S]
    public: Posn  SetEnd(Posn);
    public: void  SetRange(Posn, Posn);
    public: void  SetRange(const Range*);
    public: Posn  SetStart(Posn);
    public: void SetText(const base::string16& text);
    public: Count StartOf(Unit, bool = false);

    // [U]
    public:  void  Upcase();
}; // Range

}  // namespace text

#endif //!defined(INCLUDE_edit_range_h)
