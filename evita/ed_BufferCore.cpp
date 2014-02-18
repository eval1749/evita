#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Edit Buffer
// listener/winapp/ed_buffer.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_BufferCore.cpp#2 $
//
#include "evita/ed_BufferCore.h"

#include <algorithm>

#include "evita/li_util.h"

#define DEBUG_LIFE 0

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// Buffer ctor
//
BufferCore::BufferCore() :
    m_cwch(MIN_GAP_LENGTH * 3),
    m_lEnd(0),
    m_lGapEnd(m_cwch),
    m_lGapStart(0)
{
    m_hHeap = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
    ASSERT(NULL != m_hHeap);

    #if DEBUG_LIFE
      DEBUG_PRINTF("%p: new heap=%p\n", this, m_hHeap);
    #endif

    m_pwch = reinterpret_cast<char16*>(
        ::HeapAlloc(m_hHeap, 0, sizeof(char16) * m_cwch) );
} // BufferCore::BufferCore


//////////////////////////////////////////////////////////////////////
//
// BufferCore dtor
//
BufferCore::~BufferCore()
{
    #if DEBUG_LIFE
      DEBUG_PRINTF("%p: heap=%p\n", this, m_hHeap);
    #endif
    if (NULL != m_hHeap) ::HeapDestroy(m_hHeap);
} // BufferCore::~BufferCore


//////////////////////////////////////////////////////////////////////
//
// BufferCore::deleteChars
//
Count BufferCore::deleteChars(Posn lStart, Posn lEnd)
{
    ASSERT(IsValidRange(lStart, lEnd));

    Count n = lEnd - lStart;

    moveGap(lStart);
    m_lGapEnd += n;
    m_lEnd -= n;

    return n;
} // BufferCore::deleteChars


//////////////////////////////////////////////////////////////////////
//
// Extend Buffer And Make Gap
//
// Called by:
//  CBufferCore::InsertString
//
void BufferCore::extend(Posn lPosn, long cwchExtent)
{
    if (cwchExtent <= 0) return;

    moveGap(lPosn);

    if ((m_lGapEnd - m_lGapStart) >= cwchExtent + MIN_GAP_LENGTH)
    {
        // We have enough GAP.
        return;
    } // if

    Count nExtension = cwchExtent + EXTENSION_LENGTH - 1;
        nExtension /= EXTENSION_LENGTH;
        nExtension *= EXTENSION_LENGTH;

    m_cwch += nExtension;

    // Extend character buffer
    // warning C6308: 'realloc' might return null pointer: assigning null pointer to 'm_pwch', which is passed as an argument to 'realloc', will cause the original memory block to be leaked
    #pragma warning(suppress: 6308)
    m_pwch = static_cast<char16*>(
        ::HeapReAlloc(m_hHeap, 0, m_pwch, sizeof(char16) * m_cwch) );

    ASSERT(NULL != m_pwch);

    // Extend GAP
    myMoveMemory(
        m_pwch + m_lGapEnd + nExtension,
        m_pwch + m_lGapEnd,
        sizeof(char16) * (m_lEnd - m_lGapStart) );

    m_lGapEnd += nExtension;
} // BufferCore::extend


//////////////////////////////////////////////////////////////////////
//
// BufferCore::GetCharAt
//
char16 BufferCore::GetCharAt(Posn lPosn) const
{
    ASSERT(IsValidPosn(lPosn));

    if (lPosn >= GetEnd()) return 0;

    if (lPosn >= m_lGapStart) 
    {
        lPosn += m_lGapEnd - m_lGapStart;
    }

    return m_pwch[lPosn];
} // BufferCore::GetCharAt


//////////////////////////////////////////////////////////////////////
//
// BufferCore::GetText
//
Count BufferCore::GetText(char16* prgwch, Posn lStart, Posn lEnd) const
{
    if (lStart < 0) lStart = 0;
    if (lEnd > GetEnd()) lEnd = GetEnd();
    if (lStart >= lEnd) return 0;

    if (lStart >= m_lGapStart)
    {
        // We extract text after gap.
        // gggggg<....>
        myCopyMemory(
            prgwch,
            m_pwch + m_lGapEnd + (lStart - m_lGapStart),
            sizeof(char16) * (lEnd - lStart) );
    }
    else
    {
        // We extract text before gap.
        // <.....>gggg
        // <...ggg>ggg
        // <...ggg...>
        Posn lMiddle = std::min(m_lGapStart, lEnd);

        myCopyMemory(
            prgwch,
            m_pwch + lStart,
            sizeof(char16) * (lMiddle - lStart) );

        myCopyMemory(
            prgwch + (lMiddle - lStart),
            m_pwch + m_lGapEnd,
            sizeof(char16) * (lEnd - lMiddle) );
    } // if

    return lEnd - lStart;
} // BufferCore::GetText

base::string16 BufferCore::GetText(Posn start, Posn end) const {
    if (start < 0)
      start = std::max(GetEnd() + start, static_cast<Posn>(0));
    else if (start > GetEnd())
      start = GetEnd();
    if (end < 0)
      end = std::max(GetEnd() + end, static_cast<Posn>(0));
    else if (end > GetEnd())
      end = GetEnd();
    auto const cwch = end - start;
    if (cwch <= 0)
      return base::string16();
    base::string16 text(static_cast<size_t>(cwch), ' ');
    GetText(&text[0], start, end);
    return std::move(text);
}

//////////////////////////////////////////////////////////////////////
//
// BufferCore::insert
//
// Description:
//  Inserts specified string (pwch, n) before lPosn.
//
void BufferCore::insert(Posn lPosn, const char16* pwch, Count n)
{
    ASSERT(IsValidPosn(lPosn));

    extend(lPosn, n);

    myCopyMemory(m_pwch + lPosn, pwch, sizeof(char16) * n);

    m_lGapStart += n;
    m_lEnd += n;
} // BufferCore::insert


//////////////////////////////////////////////////////////////////////
//
// Move Gap To Specified Position
//
// Called by:
//  CBufferCore::delteChars
//  CBufer::insertString
//
// User 1 2 3 4 5 6 7 8           9 A B
//       M i n n e a p o _ _ _ _ _ l i s 
// Gap  1 2 3 4 5 6 7 8 9 A B C D E F 10
//              Posn     Gap
 
void
BufferCore::moveGap(Posn lNewStart)
{
    Posn lCurEnd   = m_lGapEnd;
    Posn lCurStart = m_lGapStart;

    int iDiff = m_lGapStart - lNewStart;

    Posn lNewEnd = m_lGapEnd - iDiff;

    m_lGapEnd   = lNewEnd;
    m_lGapStart = lNewStart;

    if (iDiff > 0)
    {
        ////////////////////////////////////////////////////////////
        //
        // Move GAP backward =
        //  Move GAP between lNewStart and lCurStart before lCurEnd.
        // abcdef....ghijk
        //    ^  s   e
        // abc....defghijk
        //    s   e
        myMoveMemory(
            m_pwch + lNewEnd,
            m_pwch + lNewStart,
            sizeof(char16) * iDiff );
    }
    else if (iDiff < 0)
    {
        ////////////////////////////////////////////////////////////
        //
        // Move GAP forward =
        //  Move string between lCurEnd and m_lGapEnd after lCurStart.
        // abcde...fghijk
        //      s  e   ^
        //         |   |
        //      +--+   |
        //      |      |
        //      |   +--+
        //      V   V
        // abcdefghi...jk
        //          s  e
        myMoveMemory(
            m_pwch + lCurStart,
            m_pwch + lCurEnd,
            sizeof(char16) * -iDiff );
    } // if
} // BufferCore::moveGap

}  // namespace text
