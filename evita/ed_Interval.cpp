#include "precomp.h"
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
#include "evita/ed_Interval.h"
#include "evita/text/buffer.h"

extern StyleValues g_DefaultStyle;

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
    m_Style(g_DefaultStyle) {}

/// <summary>
///   Set style of interval.
/// </summary>
void Interval::SetStyle(const StyleValues* p)
{
    #define mergeStyle(mp_field) \
        if (p->m_rgfMask & StyleValues::Mask_##mp_field) \
        { \
            m_Style.m_rgfMask |= StyleValues::Mask_##mp_field; \
            m_Style.Set##mp_field(p->Get##mp_field()); \
        }

    Posn lPosn = GetEnd();

    mergeStyle(Background);
    mergeStyle(Color);
    mergeStyle(Decoration);
    mergeStyle(FontFamily);
    mergeStyle(FontSize);
    mergeStyle(FontStyle);
    mergeStyle(FontWeight);
    mergeStyle(Marker);
    mergeStyle(Syntax);

    #undef mergeStyle

    DCHECK_EQ(GetEnd(), lPosn);
} // Interval::SetStyle

/// <summary>
///   Retreive interval including specified position.
/// </summary>
/// <param name="lPosn">A postion for an interval to get.</param>
/// <returns>An Interval object</returns>
Interval* Buffer::GetIntervalAt(Posn lPosn) const
{
    lPosn = std::min(lPosn, GetEnd());

    Interval* pRunner = m_oIntervalTree.GetRoot();
    while (NULL != pRunner)
    {
        if (pRunner->Contains(lPosn))
        {
            break;
        }

        if (lPosn < pRunner->GetStart())
        {
            pRunner = pRunner->GetLeft();
        }
        else
        {
            pRunner = pRunner->GetRight();
        }
    } // while

    #if DEBUG_STYLE
    {
        DEBUG_PRINTF(L"%p lPosn=%d\n", this, lPosn);
        int nNth = 0;
        foreach (EnumInterval, oEnum, this)
        {
            Interval* pIntv = oEnum.Get();
            DEBUG_PRINTF(L"%d %p [%d, %d] #%06X\n",
                nNth, pIntv, pIntv->GetStart(), pIntv->GetEnd(),
                pIntv->m_Style.m_crColor );
            nNth += 1;
        } // for each interval
    }
    #endif // DEBUG_STYLE

    if (NULL == pRunner)
    {
        const Interval* pFound = NULL;
        foreach (Intervals::Enum, oEnum, &m_oIntervals)
        {
            if (oEnum.Get()->Contains(lPosn))
            {
                pFound = oEnum.Get();
                break;
            }
        } // for

        if (!pFound) {
          LOG(0) << "position=" << lPosn << " insnt' in list.";
        } else {
          LOG(0) << "position=" << lPosn << " is in list intv[" <<
            pFound->GetStart() << "," << pFound->GetEnd() << "]";
        }
        NOTREACHED();
    } // if

    return pRunner;
} // Buffer::GetIntervalAt

/// <summary>
///   Set style of range of buffer.
/// </summary>
/// <param name="lEnd">An end position (exlclude).</param>
/// <param name="lStart">A start position (include).</param>
/// <param name="pStyle">A style to set</param>
void Buffer::SetStyle(
    Posn                lStart,
    Posn                lEnd,
    const StyleValues*  pStyle )
{
    DCHECK(pStyle);

    #if DEBUG_STYLE
        DEBUG_PRINTF(L"%p [%d, %d] color=#x%06X\n",
            this, lStart, lEnd, pStyle->GetColor() );
    #endif // DEBUG_STYLE

    if (lStart < 0) lStart = 0;
    if (lEnd > GetEnd()) lEnd = GetEnd();
    if (lStart >= lEnd) return;

    // To improve performance, we don't check contents of pStyle.
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
        pHead->SetStyle(pStyle);
        tryMergeInterval(pHead);
        return;
    }

    if (lHeadEnd < lEnd)
    {
        // pHead: --s...e----
        // Range: ----s.....e----
        SetStyle(lStart,   lHeadEnd, pStyle);
        SetStyle(lHeadEnd, lEnd,     pStyle);
        return;
    } // if

    // New style is compatibile with existing one.
    Interval oIntv(lStart, lEnd);
    oIntv.SetStyle(pStyle);
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

        Interval* pIntv = newInterval(lStart, lEnd);
        pIntv->SetStyle(pStyle);

        m_oIntervals.InsertBefore(pIntv, pHead);
        m_oIntervalTree.Insert(pIntv);

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

        Interval* pIntv = newInterval(lStart, lEnd);
        pIntv->SetStyle(pStyle);

        m_oIntervals.InsertAfter(pIntv, pHead);
        m_oIntervalTree.Insert(pIntv);

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

        Interval* pTail = newInterval(lEnd, lHeadEnd);
        pTail->SetStyle(pHead->GetStyle());

        m_oIntervals.InsertAfter(pTail, pHead);
        m_oIntervalTree.Insert(pTail);

        ASSERT(pHead->GetNext() == pTail);
        ASSERT(pTail->GetPrev() == pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pTail->GetStart());
            ASSERT(p == pTail);
        }
        #endif

        Interval* pIntv = newInterval(lStart, lEnd);
        pIntv->SetStyle(pStyle);

        m_oIntervals.InsertAfter(pIntv, pHead);
        m_oIntervalTree.Insert(pIntv);

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
///   Create a new interval for specified range.
/// </summary>
/// <param name="lEnd">An end position (exlclude).</param>
/// <param name="lStart">A start position (include).</param>
Interval* Buffer::newInterval(Posn lStart, Posn lEnd) {
  DCHECK_LE(lStart, lEnd);
  return new(m_hObjHeap) Interval(lStart, lEnd);
}

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

            m_oIntervals.Delete(pIntv);
            m_oIntervalTree.Delete(pIntv);

            destroyObject(pIntv);

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

            m_oIntervals.Delete(pNext);
            m_oIntervalTree.Delete(pNext);

            destroyObject(pNext);

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
    const StyleValues* p = GetStyle();
    const StyleValues* q = pIntv->GetStyle();

    if (p->m_rgfMask != q->m_rgfMask)
    {
        return false;
    }

    #define compare(mp_name) \
        if (p->m_rgfMask & StyleValues::Mask_##mp_name && \
            p->Get##mp_name() != q->Get##mp_name() ) \
        { \
            return false; \
        }

    compare(Background)
    compare(Color)
    compare(Decoration)
    compare(FontSize)
    compare(FontStyle)
    compare(FontWeight)
    compare(Marker)
    compare(Syntax)

    #undef compare

    if (p->m_rgfMask & StyleValues::Mask_FontFamily)
    {
        if (::lstrcmpW(p->m_pwszFontFamily, q->m_pwszFontFamily))
        {
            return false;
        }
    }

    return true;
} // Interval::CanMerge

}  // namespace text
