//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Listern
// listener/winapp/vi_Listner.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Listener.h#1 $
//
#if !defined(INCLUDE_listener_winapp_visual_listener_h)
#define INCLUDE_listener_winapp_visual_listener_h

namespace dom {
class Document;
}

using Buffer = dom::Document;

namespace Edit
{
    class Range;
} // Edit

//////////////////////////////////////////////////////////////////////
//
// Listener
//
class Listener
{
    private: typedef Edit::Range Range;

    private: struct SendInfo
    {
        Posn    m_lEnd;
        Posn    m_lPosn;
    }; // SendInfo

    private: DWORD      m_dwThread;
    private: SendInfo   m_oSendInfo;
    private: Buffer*    m_pBuffer;
    private: Range*     m_pOutput;
    private: char16     m_rgwch[1024];

    // ctor
    public: Listener(Buffer*);

    // [G]
    public: Buffer* GetBuffer() const { return m_pBuffer; }

    // [O]
    public: void OnReady();
    public: void OnReceiveText(const char16*, int);
    public: void OnSentText(Count);

    // [Q]
    public: Count QueryColumn() const;

    // [S]
    public:  void SendText();
    private: void sendTextAux();
}; // Listener

#endif //!defined(INCLUDE_listener_winapp_visual_listener_h)
