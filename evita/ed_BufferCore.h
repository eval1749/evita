//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_BufferCore.h#1 $
//
#if !defined(INCLUDE_edit_BufferCore_h)
#define INCLUDE_edit_BufferCore_h

#include "base/strings/string16.h"

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// BufferCore
//
class BufferCore
{
    private: static const Count MIN_GAP_LENGTH    = 1024;
    private: static const Count EXTENSION_LENGTH  = 1024;

    private: char16*    m_pwch;
    private: Count      m_cwch;
    private: HANDLE     m_hHeap;
    private: Posn       m_lEnd;
    private: Posn       m_lGapEnd;
    private: Posn       m_lGapStart;

    protected: BufferCore();
    public:    ~BufferCore();

    public: bool operator==(const BufferCore* other) const {
        return this == other;
    }

    public: bool operator!=(const BufferCore* other) const {
        return this != other;
    }

    // [D]
    protected: Count deleteChars(Posn, Posn);

    // [E]
    public: Posn EnsurePosn(Posn lPosn) const
    {
        if (lPosn < 0) return 0;
        if (lPosn > m_lEnd) return m_lEnd;
        return lPosn;
    } // EnsurePosn

    protected: void extend(Posn, long);

    // [G]
    public: char16 GetCharAt(Posn) const;
    public: Posn   GetEnd() const { return m_lEnd; }
    public: Count  GetText(char16*, Posn, Posn) const;
    public: base::string16 GetText(Posn start, Posn end) const;

    // [I]
    protected: void insert(Posn, char16, Count);
    protected: void insert(Posn, const char16*, Count);

    public: bool IsValidPosn(Posn p) const
        { return p >= 0 && p <= m_lEnd; }

    public: bool IsValidRange(Posn s, Posn e) const
        { return IsValidPosn(s) && IsValidPosn(e) && s <= e; }

    // [M]
    private:  bool Match(Posn, const char16*, int, uint) const;
    private: void moveGap(Posn);
}; // BufferCore

}  // namespace text

#endif //!defined(INCLUDE_edit_BufferCore_h)
