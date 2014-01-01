//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_listener_buffer.h#1 $
//
#if !defined(INCLUDE_listener_winapp_app_listener_buffer_h)
#define INCLUDE_listener_winapp_app_listener_buffer_h

#include "./ap_input_history.h"
#include "./ap_lisp_thread.h"
#include "./vi_Gateway.h"

#include "evita/dom/buffer.h"

class Selection;

namespace Command
{
    class KeyBindEntry;
    class KeyBinds;
} // Command

//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer
//
class ListenerBuffer :
    public Buffer
{
    public: virtual Command::KeyBindEntry* MapKey(uint) const;

    private: static Command::KeyBinds* sm_pKeyBinds;

    public: Gateway         m_oGateway;
    public: InputHistory    m_oHistory;
    private: LispThread     m_oLispThread;

    // ctor/dtor
    public: ListenerBuffer();
    public: ~ListenerBuffer() {}

    // [A]
    public: void AdjustSelection(Selection*);

    // [I]
    public: void Interrupt();

    // [O]
    public: virtual bool OnIdle();

    // [S]
    public: void SendText();
    public: uint  Start();
}; // ListenerBuffer

#endif //!defined(INCLUDE_listener_winapp_app_listener_buffer_h)
