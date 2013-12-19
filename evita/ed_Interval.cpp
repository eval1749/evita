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
#include "./ed_Interval.h"

#include "./ed_Buffer.h"

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

namespace Edit
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

    ASSERT(GetEnd() == lPosn);
} // Interval::SetStyle

/// <summary>
///   Retreive interval including specified position.
/// </summary>
/// <param name="lPosn">A postion for an interval to get.</param>
/// <returns>An Interval object</returns>
Interval* Buffer::GetIntervalAt(Posn lPosn) const
{
    lPosn = min(lPosn, GetEnd());

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

        char16 wsz[200];
        if (NULL == pFound)
        {
            ::wsprintfW(wsz, L"posn=%d isn't in list.", lPosn);
        }
        else
        {
            ::wsprintf(wsz,
                L"posn=%d is in list intv[%d, %d].",
                lPosn,
                pFound->GetStart(),
                pFound->GetEnd() );
        }

        if (::IsDebuggerPresent())
        {
            ::MessageBox(NULL, wsz, L"Evita Editor", MB_ICONERROR);
            ::DebugBreak();
        }
        else
        {
            ::FatalAppExit(0, wsz);
        }
        // NOTREACHED
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
    ASSERT(pStyle != NULL);

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

        ASSERT(pHead->GetPrev() == pIntv);
        ASSERT(pIntv->GetNext() == pHead);

        #if DEBUG_INTERVAL
        {
            Interval* p = GetIntervalAt(pIntv->GetStart());
            ASSERT(p == pIntv);
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

        ASSERT(pHead->GetNext() == pIntv);
        ASSERT(pIntv->GetPrev() == pHead);

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
Interval* Buffer::newInterval(Posn lStart, Posn lEnd)
{
    unless (lStart <= lEnd)
    {
        char16 wsz[100];
        ::wsprintfW(wsz, L"newInterval: Bad range %d, %d",
            lStart, lEnd );
        ::FatalAppExit(0, wsz);
        // NOTREACHED
    }

    Interval* pIntv= new(m_hObjHeap) Interval(lStart, lEnd);
    return pIntv;
} // Buffer::newInterval

/// <summary>
///   Try merge specified interval to previous and next.
/// </summary>
Interval* Buffer::tryMergeInterval(Interval* pIntv)
{
    // Merge to previous
    if (Interval* pPrev = pIntv->GetPrev())
    {
        ASSERT(pPrev->GetEnd()  == pIntv->GetStart());
        ASSERT(pPrev->GetNext() == pIntv);

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
        ASSERT(pIntv->GetEnd() == pNext->GetStart());

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

void BufPrintf(Buffer* pBuffer, const char16* pwszFormat, ...)
{
    char16 wsz[256];
    va_list args;
    va_start(args, pwszFormat);
    ::wvsprintfW(wsz, pwszFormat, args);
    va_end(args);
    pBuffer->Insert(pBuffer->GetEnd(), wsz);
} // BufPrintf

bool Buffer::ValidateIntervals(Buffer* pLogBuf) const
{
    bool fValid = true;

    uint cIntvs = 0;

    {
        Posn lLast = 0;
        foreach (Intervals::Enum, oEnum, &m_oIntervals)
        {
            Interval* pIntv = oEnum.Get();
            if (pIntv->GetStart() != lLast)
            {
                fValid = false;

                BufPrintf(pLogBuf, L"Start of [%d, %d] must be %d\n",
                    pIntv->GetStart(),
                    pIntv->GetEnd(),
                    lLast );
            }

            if (pIntv->GetStart() >= pIntv->GetEnd())
            {
                fValid = false;

                BufPrintf(pLogBuf, L"Empty [%d, %d]\n",
                    pIntv->GetStart(),
                    pIntv->GetEnd() );
            }

            if (pIntv->GetEnd() > GetEnd() + 1)
            {
                fValid = false;

                BufPrintf(pLogBuf, L"Outside [%d, %d]\n",
                    pIntv->GetStart(),
                    pIntv->GetEnd() );
            }

            lLast = pIntv->GetEnd();

            cIntvs += 1;
        } // for each interval
    }

    BufPrintf(pLogBuf, L"Buffer '%s' has %d intervals.\n",
        GetName(),
        cIntvs );

    for (Posn lPosn = 0; lPosn < GetEnd(); lPosn += 1)
    {
        Interval* pRunner = m_oIntervalTree.GetRoot();
        Interval* pFound  = NULL;
        while (NULL != pRunner)
        {
            if (pRunner->Contains(lPosn))
            {
                pFound = pRunner;
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

        if (NULL == pFound)
        {
            fValid = false;
            BufPrintf(pLogBuf, L"Posn %d isn't in tree.\n", lPosn);
        }
    } // for all posn


    class TreeWalker
    {
        public: static bool Walk(Interval* pIntv, Buffer* pLogBuf)
        {
            when (NULL == pIntv) return true;

            bool fValid = true;

            if (Interval* pLeft = pIntv->GetLeft())
            {
                unless (pLeft->GetEnd() <= pIntv->GetStart())
                {
                    fValid = false;
                    BufPrintf(pLogBuf, L"left [%d, %d] must be %d.\n",
                        pLeft->GetStart(),
                        pLeft->GetEnd(),
                        pIntv->GetStart() );
                }

                unless (Walk(pLeft, pLogBuf)) fValid = false;
            }

            if (Interval* pRight = pIntv->GetRight())
            {
                unless (pRight->GetStart() >= pIntv->GetEnd())
                {
                    fValid = false;
                    BufPrintf(pLogBuf, L"right [%d, %d] must be %d.\n",
                        pRight->GetStart(),
                        pRight->GetEnd(),
                        pIntv->GetStart() );
                }

                unless (Walk(pRight, pLogBuf)) fValid = false;
            }

            return fValid;
        } // Walk
    }; // TreeWalker

    unless (TreeWalker::Walk(m_oIntervalTree.GetRoot(), pLogBuf))
    {
        fValid = false;
    }

    return fValid;
} // Buffer::ValiateIntervals

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

} // Edit
