//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_DialogBox.h#1 $
//
#if !defined(INCLUDE_visual_dialog_h)
#define INCLUDE_visual_dialog_h

//////////////////////////////////////////////////////////////////////
//
// DialogBox
//
class DialogBox
{
    protected: HWND                 m_hwnd;
    protected: static DialogBox*    sm_pCreate;

    // operator
    public: operator HWND() const { return m_hwnd; }

    // [D]
    protected: static INT_PTR CALLBACK dialogProc(HWND, UINT, WPARAM, LPARAM);

    public: void DoModal(HWND);
    public: bool DoModeless(HWND = NULL);

    // [G]
    public: bool GetChecked(int iCtrl) const
    {
        LRESULT lResult = ::SendMessage(
            GetDlgItem(iCtrl),
            BM_GETCHECK,
            0,
            0 );
        return BST_CHECKED == lResult;
    } // GetChecked

    public: HWND GetDlgItem(int iCtrl) const
        { return ::GetDlgItem(m_hwnd, iCtrl); }

    protected: virtual int  GetTemplate() const = 0;

    // [I]
    public: bool IsRealized() const { return NULL != m_hwnd; }

    // [O]
    protected: virtual bool onCommand(WPARAM, LPARAM) { return false; }
    protected: virtual bool onInitDialog() = 0;
    protected: virtual void onOk();
    protected: virtual void onCancel();
    protected: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM) { return 0; }

    // [S]
    public: int SetCheckBox(int iCtrl, bool fChecked)
    {
        return static_cast<int>(::SendMessage(
            GetDlgItem(iCtrl),
            BM_SETCHECK,
            static_cast<WPARAM>(fChecked ? BST_CHECKED : BST_UNCHECKED),
            0 ) );
    } // SetCheckBox
}; // Dialog

#endif //!defined(INCLUDE_visual_dialog_h)
