//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_input_history.h#1 $
//
#if !defined(INCLUDE_listener_winapp_app_input_history_h)
#define INCLUDE_listener_winapp_app_input_history_h

class Selection;

//////////////////////////////////////////////////////////////////////
//
// InputHistory
//
class InputHistory
{
    private: uint m_nEnd;
    private: uint m_nCurr;
    private: char16* m_rgpwsz[30];

    public: InputHistory();
    public: void Add(Edit::Buffer*, Posn, Posn);
    public: void Backward(Selection*);
    public: void Forward(Selection*);
}; // InputHistory


#endif //!defined(INCLUDE_listener_winapp_app_input_history_h)
