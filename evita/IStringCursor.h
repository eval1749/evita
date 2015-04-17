//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/IStringCursor.h#1 $
//
#if !defined(INCLUDE_IStringCursor)
#define INCLUDE_IStringCursor

#include "base/basictypes.h"
#include "evita/precomp.h"

struct IStringCursor
{
    // [C]
    virtual bool   CanMove(int) = 0;

    // [G]
    virtual char16 GetChar(int) = 0;
    virtual long   GetPosition() = 0;

    // [F]
    virtual bool FindBackward(char16, uint32_t) = 0;
    virtual bool FindForward(char16, uint32_t) = 0;

    // [M]
    virtual bool Match(const char16*, int, uint32_t) = 0;
    virtual long Move(int) = 0;
    virtual long MoveToEnd() = 0;

    // [S]
    virtual long MoveToStart() = 0;
}; // IStringCursor


struct StringRange
{
    int m_lStart;
    int m_lEnd;
}; // StringRange

struct IStringMatcher
{
    virtual bool FirstMatch(IStringCursor*) = 0;
    virtual bool GetMatched(int, StringRange*) = 0;
    virtual bool NextMatch() = 0;
}; // IStringMatcher

#endif //!defined(INCLUDE_IStringCursor)
