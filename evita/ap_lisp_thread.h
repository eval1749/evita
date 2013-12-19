//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ap_lisp_thread.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_lisp_thread.h#1 $
//
#if !defined(INCLUDE_listener_winapp_app_lisp_thread_h)
#define INCLUDE_listener_winapp_app_lisp_thread_h

// LispThread
class LispThread
{
    private: DWORD          m_dwThread;
    private: HMODULE        m_hDll;
    private: HANDLE         m_hThread;
    private: HWND           m_hwnd;
    private: void*          m_pvLisp;

    public: LispThread();
    public: ~LispThread();

    // [G]
    public: DWORD GetId() const { return m_dwThread; }

    // [I]
    public: uint Init();
    public: void Interrupt();

    // [S]
    public: uint Start(HWND);

    // [L]
    private: DWORD threadProc();

    private: static DWORD WINAPI threadProc_(void* pv)
        { return reinterpret_cast<LispThread*>(pv)->threadProc(); }
}; // LispThread

#endif //!defined(INCLUDE_listener_winapp_app_lisp_thread_h)
