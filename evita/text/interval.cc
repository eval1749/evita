//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - editor - interval
// listener/winapp/ed_interval.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Interval.cpp#4 $
//
#define DEBUG_INTERVAL  _DEBUG
#define DEBUG_STYLE     0

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/interval_set.h"

///
/// <summary>
///   Returns random number for Treap.
/// </summary>
int TreapRandom()
{
    DEFINE_STATIC_LOCAL(uint32, s_nRandom, (::GetTickCount()));
    s_nRandom =  s_nRandom * 1664525 + 1013904223;
    return static_cast<int>(s_nRandom & ((1<<28)-1)) & MAXINT;
} // TreapRandom

namespace text
{

/// <summary>
///   Construct an interval object at specified range.
/// </summary>
Interval::Interval(Posn lStart, Posn lEnd, int nZ) :
    m_lEnd(lEnd),
    m_lStart(lStart),
    m_nZ(nZ),
    m_Style(*css::Style::Default()) {}

/// <summary>
///   Set style of interval.
/// </summary>
void Interval::SetStyle(const css::Style& other)
{
    m_Style.OverrideBy(other);
} // Interval::SetStyle

/// <summary>
///   Retreive interval including specified position.
/// </summary>
/// <param name="lPosn">A postion for an interval to get.</param>
/// <returns>An Interval object</returns>
Interval* Buffer::GetIntervalAt(Posn lPosn) const {
  return intervals_->GetIntervalAt(std::min(lPosn, GetEnd()));
}

/// <summary>
///   Set style of range of buffer.
/// </summary>
/// <param name="lEnd">An end position (exlclude).</param>
/// <param name="lStart">A start position (include).</param>
/// <param name="style">A style to set</param>
void Buffer::SetStyle(
    Posn                lStart,
    Posn                lEnd,
    const css::Style&  style)
{
    if (lStart < 0) lStart = 0;
    if (lEnd > GetEnd()) lEnd = GetEnd();
    if (lStart >= lEnd) return;

    // To improve performance, we don't check contents of |style|.
    // This may be enough for syntax coloring.
    m_nModfTick += 1;

    // Get interval pHead containing lStart.
    Interval* pHead = GetIntervalAt(lStart);

    Posn lHeadEnd   = pHead->GetEnd();
    Posn lHeadStart = pHead->GetStart();

    if (lHeadStart == lStart && lHeadEnd == lEnd)
    {
        // pHead: ---s......e---
        // Range: ---s......e---
        pHead->SetStyle(style);
        tryMergeInterval(pHead);
        return;
    }

    if (lHeadEnd < lEnd)
    {
        // pHead: --s...e----
        // Range: ----s.....e----
        SetStyle(lStart,   lHeadEnd, style);
        SetStyle(lHeadEnd, lEnd,     style);
        return;
    } // if

    // New style is compatibile with existing one.
    Interval oIntv(lStart, lEnd);
    oIntv.SetStyle(style);
    if (oIntv.CanMerge(pHead))
    {
        return;
    }

    if (lHeadStart == lStart)
    {
        // pHead: ---s........e---
        // pTail: --------s...e---
        // Range: ---s....e-------
        pHead->m_lStart = lEnd;

        if (Interval* pPrev = pHead->GetPrev())
        {
            if (oIntv.CanMerge(pPrev))
            {
                pPrev->m_lEnd = lEnd;
                return;
            }
        }

        auto const pIntv = new Interval(lStart, lEnd);
        pIntv->SetStyle(style);

        intervals_->InsertBefore(pIntv, pHead);

        DCHECK_EQ(pHead->GetPrev(), pIntv);
        DCHECK_EQ(pIntv->GetNext(), pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pIntv->GetStart());
            DCHECK_EQ(p, pIntv);
        }
        #endif

        return;
    } // if

    if (lHeadEnd == lEnd)
    {
        // pHead: ---s........e---
        // Range: -------s....e---
        pHead->m_lEnd = lStart;

        if (Interval* pNext = pHead->GetNext())
        {
            if (oIntv.CanMerge(pNext))
            {
                pNext->m_lStart = lStart;
                return;
            }
        }

        auto const pIntv = new Interval(lStart, lEnd);
        pIntv->SetStyle(style);

        intervals_->InsertAfter(pIntv, pHead);

        DCHECK_EQ(pHead->GetNext(), pIntv);
        DCHECK_EQ(pIntv->GetPrev(), pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pIntv->GetStart());
            ASSERT(p == pIntv);
        }
        #endif

        return;
    } // if

    //{
        // pHead: ---s...........e---
        // pTail: ----------s....e---
        // Range: -----s....e--------
        pHead->m_lEnd = lStart;

        auto const pTail = new Interval(lEnd, lHeadEnd);
        pTail->SetStyle(pHead->GetStyle());

        intervals_->InsertAfter(pTail, pHead);

        ASSERT(pHead->GetNext() == pTail);
        ASSERT(pTail->GetPrev() == pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pTail->GetStart());
            ASSERT(p == pTail);
        }
        #endif

        auto const pIntv = new Interval(lStart, lEnd);
        pIntv->SetStyle(style);

        intervals_->InsertAfter(pIntv, pHead);

        ASSERT(pHead->GetNext() == pIntv);
        ASSERT(pIntv->GetPrev() == pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pIntv->GetStart());
            ASSERT(p == pIntv);
        }
        #endif

        return;
    //}
} // Buffer::SetStyle

/// <summary>
///   Try merge specified interval to previous and next.
/// </summary>
Interval* Buffer::tryMergeInterval(Interval* pIntv)
{
    // Merge to previous
    if (Interval* pPrev = pIntv->GetPrev())
    {
        DCHECK_EQ(pPrev->GetEnd(), pIntv->GetStart());
        DCHECK_EQ(pPrev->GetNext(), pIntv);

        if (pIntv->CanMerge(pPrev))
        {
            Posn lEnd = pIntv->GetEnd();

            #if 0 && DEBUG_INTERVAL
                DEBUG_PRINTF(L"delete Intv %p [%d, %d]\n",
                    pIntv, pIntv->GetStart(), pIntv->GetEnd() );
            #endif

            intervals_->RemoveInterval(pIntv);
            delete pIntv;

            pIntv = pPrev;

            pIntv->m_lEnd = lEnd;
        }
    } // if

    // Absobe next
    if (Interval* pNext = pIntv->GetNext())
    {
        DCHECK_EQ(pIntv->GetEnd(), pNext->GetStart());

        if (pIntv->CanMerge(pNext))
        {
            Posn lEnd = pNext->GetEnd();

            #if 0 && DEBUG_INTERVAL
                DEBUG_PRINTF(L"delete Next %p [%d, %d]\n",
                    pNext, pNext->GetStart(), pNext->GetEnd() );
            #endif

            intervals_->RemoveInterval(pNext);
            delete pNext;

            pIntv->m_lEnd = lEnd;
        }
    } // if

    return pIntv;
} // Buffer::tryMergeInterval

/// <summary>
///   Returns true if this interval and pIntv can be merged. If two
///   intervals are mergeable, both intervals have equivalent styles.
/// </summary>
bool Interval::CanMerge(const Interval* pIntv) const
{
    return GetStyle() == pIntv->GetStyle();
} // Interval::CanMerge

}  // namespace text
