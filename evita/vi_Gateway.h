//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Process Gateway
// listener/winapp/ed_gateway.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Gateway.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_gateway_h)
#define INCLUDE_listener_winapp_visual_gateway_h

class Buffer;

//////////////////////////////////////////////////////////////////////
//
// Gateway
//
// FIXME 2007-06-10 REVIEW: Can we use only one message window for all
// threads?
//
class Gateway
{
    private: typedef Edit::Range Range;

    private: struct SendInfo
    {
        Posn    m_lEnd;
        Posn    m_lPosn;
    }; // SendInfo

    private: static ATOM     sm_atomWndClass;
    private: static Gateway* sm_pCreateWnd;

    private: DWORD      m_dwThread;
    private: HWND       m_hwnd;
    private: SendInfo   m_oSendInfo;
    private: Buffer*    m_pBuffer;
    private: Range*     m_pOutput;
    private: char16     m_rgwch[1024];

    // Gateway ctor
    public: Gateway();

    // [G]
    public: Buffer* GetBuffer() const { return m_pBuffer; }
    public: Range*  GetOutput() const { return m_pOutput; }
    public: HWND    GetHWND()   const { return m_hwnd; }

    // [O]
    protected: virtual LRESULT onMessage(uint, WPARAM, LPARAM);

    // [S]
    public:  void SendText();
    private: void sendTextAux();
    public:  uint Start(Buffer*, DWORD);

    // [W]
    private: static LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);
}; // Gateway

#endif //!defined(INCLUDE_listener_winapp_visual_gateway_h)
