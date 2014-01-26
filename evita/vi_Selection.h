//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Selection.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_selection_h)
#define INCLUDE_listener_winapp_visual_selection_h

#include "evita/vi_defs.h"

#include "evita/text/range.h"
#include "evita/ed_Style.h"
#include "evita/gfx_base.h"

enum SelectionType
{
    Selection_None,
    Selection_Normal,
}; // SelectionType


class TextEditWindow;

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
//  m_lRestoreLineNum
//   A line number which selection belongs before reloading. We try to
//   move selection to this line after reloading.
//
class Selection : public text::Range
{
    private: typedef text::Range  Range;

    private: Color              m_crColor;
    private: Color              m_crBackground;
    private: bool               m_fStartIsActive;
    private: Count              m_lRestoreLineNum;  // for reloading
    private: Buffer*            m_pBuffer;
    private: gfx::PointF        m_ptGoal; // Goal point of vertical motion
    private: TextEditWindow*    m_pWindow;

    // ctor
    private : explicit Selection(const text::Range& range);
    public: Selection(TextEditWindow*, Buffer*);
    public: ~Selection();

    public: void set_window(TextEditWindow* window) {
        m_pWindow = window;
    }

    // [B]
    public: void Blink(Posn, Count);

    // [C]
    public:  void Collapse(CollapseWhich = Collapse_Start);
    public: static Selection* Create(const text::Range& range);

    // [E]
    public: Count EndOf(Unit, bool = false);

    // [G]
    public: Posn GetActivePosn() const
        { return m_fStartIsActive ? GetStart() : GetEnd(); }

    public: Color           GetBackground() const { return m_crBackground; }
    public: Buffer*         GetBuffer()     const { return m_pBuffer; }
    public: Color           GetColor()      const { return m_crColor; }
    public: SelectionType   GetType()       const;
    public: TextEditWindow* GetWindow()     const { return m_pWindow; }

    // [I]
    protected: bool isSingleLine() const;
    public:    bool IsStartActive() const { return m_fStartIsActive; }

    // [M]
    public:  Count MoveDown (Unit, Count, bool = false);
    public:  Count MoveLeft (Unit, Count, bool = false);
    public:  Count MoveRight(Unit, Count, bool = false);
    public:  void  MoveTo(Posn, bool = false);
    public:  Count MoveUp(Unit, Count, bool = false);

    // [P]
    public: void PrepareForReload();

    // [R]
    public: void RestoreForReload();

    // [S]
    public: Posn SetActivePosn(Posn lPosn)
    { 
        if (m_fStartIsActive)
        {
            SetStart(lPosn);
        }
        else
        {
            SetEnd(lPosn);
        }
        return lPosn;
    } // SetActivePosn

    public: Color SetBackground(Color cr) { return m_crBackground = cr; }
    public: Color SetColor(Color cr) { return m_crColor = cr; }
    public: void  SetEnd(Posn);
    public: void  SetRange(Posn, Posn);
    public: void  SetRange(const Range*);
    public: void  SetText(const char16*);
    public: void  SetText(const char16*, int);
    public: void  SetStart(Posn);
    public: void SetStartIsActive(bool new_start_is_active);
    public: Count StartOf(Unit, bool = false);

    // [T]
    public: void TypeChar(char16, Count = 1);
    public: void TypeEnter(Count);
    public: void TypeText(const char16*, Count);

    ////////////////////////////////////////////////////////////
    //
    // Methods for implementation
    //

    // [F]
    private: void  forgetGoal();

    // [M]
    private: Count moveAux(Unit, Count, bool);

    // [U]
    private: bool updateGoal();
}; // Selection

#endif //!defined(INCLUDE_listener_winapp_visual_selection_h)
