//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FindDialogBox.h#4 $
//
#if !defined(INCLUDE_visual_dialog_find_h)
#define INCLUDE_visual_dialog_find_h

#include "./IStringCursor.h"
#include "./resource.h"
#include "./vi_DialogBox.h"

class RegexMatcher;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
class FindDialogBox : public DialogBox
{
    public: enum Direction
    {
        Direction_Down,
        Direction_Up,
    }; // Direction

    public: enum ReplaceIn
    {
        ReplaceIn_Selection,
        ReplaceIn_Whole,
    }; // ReplaceIn

    protected: Direction m_eDirection;
    protected: ReplaceIn m_eReplaceIn;

    // ctor
    public: FindDialogBox();

    // [C]
    protected: void clearMessage();

    // [D]
    public:  void DoFind(Direction);
    private: void doReplace(uint);

    // [F]
    private: bool findFirst(RegexMatcher*);

    // [G]
    protected: virtual int GetTemplate() const override
        { return IDD_FIND; }

    // [O]
    protected: void            onCancel();
    protected: virtual bool   onCommand(WPARAM, LPARAM) override;
    protected: virtual bool   onInitDialog() override;
    protected: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM);
    protected: void            onFindNext();
    protected: void            onFindPrevious();
    protected: void            onOk();
    protected: void            onReplaceOne();
    protected: void            onReplaceAll();

    // [P]
    private: Selection* prepareFind(SearchParameters*);

    // [R]
    private: void reportNotFound();

    // [U]
    protected: void updateUI(bool = false);
}; // FileDialogBox

#endif //!defined(INCLUDE_visual_dialog_find_h)
