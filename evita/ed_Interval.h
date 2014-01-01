//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit interval
// listener/winapp/ed_interval.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Interval.h#2 $
//
#if !defined(INCLUDE_edit_interval_h)
#define INCLUDE_edit_interval_h

#include "./ed_BinTree.h"
#include "./ed_Style.h"
#include "./li_util.h"

namespace text
{

/// <summary>
///   An interval on buffer. An interval object contains style and syntax
///   information.
/// </summary>
class Interval  :
    public BinaryTree<Interval>::NodeBase,
    public DoubleLinkedNode_<Interval>,
    public ObjectInHeap
{
    friend class Buffer;

    private: Posn           m_lEnd;
    private: Posn           m_lStart;
    private: int            m_nZ;
    private: StyleValues    m_Style;

    // ctor
    public: Interval(Posn lStart, Posn lEnd, int nZ = 9);

    // [C]
    public: bool CanMerge(const Interval*) const;

    public: int Compare(const Interval* pThat) const
    {
        return m_lStart - pThat->m_lStart;
    } // Compare

    public: bool Contains(Posn lPosn) const
    {
        return lPosn >= m_lStart && lPosn < m_lEnd;
    } // Contains

    // [G]
    public: Posn      GetEnd()   const { return m_lEnd; }
    public: Posn      GetStart() const { return m_lStart; }
    public: const StyleValues* GetStyle() const { return &m_Style; }

    // [I]
    public: bool IsEmpty() const { return m_lStart == m_lEnd; }

    // [S]
    public: void SetStyle(const StyleValues*);
}; // Interval

}  // namespace text

#endif //!defined(INCLUDE_edit_interval_h)
