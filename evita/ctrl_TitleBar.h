//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_TitleBar.h#1 $
//
#if !defined(INCLUDE_visual_control_TitleBar_h)
#define INCLUDE_visual_control_TitleBar_h

//////////////////////////////////////////////////////////////////////
//
// TitleBar
//
class TitleBar
{
    private: int    m_cwch;
    private: HWND   m_hwnd;
    private: char16 m_wsz[100];

    // ctor
    public: TitleBar() :
        m_cwch(0),
        m_hwnd(NULL)
    {
        m_wsz[0] = 0;
    } // TitleBar

    // [I]
    public: bool IsEqual(const char16*, int) const;

    // [R]
    public: int Realize(HWND);

    // [S]
    public: int SetText(const char16*, int);
}; // TitleBar

#endif //!defined(INCLUDE_visual_control_TitleBar_h)
