//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/listener.h#1 $
//
#if !defined(INCLUDE_listener_winapp_listener_h)
#define INCLUDE_listener_winapp_listener_h

enum ListenerMessage
{
    LISTENER_WM_BEGIN = WM_USER,

    LISTENER_WM_QUERYCOLUMN,
    LISTENER_WM_SENDTEXT,
    LISTENER_WN_READY,
    LISTENER_WN_RECEIVETEXT,

    LISTENER_WM_LIMIT,
}; // ListenerMessage

struct ListenerInfo
{
    HWND    m_hwndListener;
    HANDLE  m_hStdOut;
    void*   m_pvListener;
}; // ListenerInfo

struct ListenerText
{
    void*   m_pvListener;
    int     m_cwch;
}; // ListenerText

typedef HWND (__fastcall *ListenerBindFn)(ListenerInfo*);
typedef int (__fastcall *StartFn)(void);

#endif //!defined(INCLUDE_listener_winapp_listener_h)
