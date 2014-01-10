//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_TabBand.h#1 $
//
#if !defined(INCLUDE_control_tabBand_h)
#define INCLUDE_control_tabBand_h

#define TABBAND_NOTIFY_CLICK_CLOSE_BUTTON (TCN_LAST - 1)

enum TabBandDragAndDrop {
    kHover,
    kDrop,
    kThrow,
};

struct TabBandNotifyData : NMHDR {
  int tab_index_;
  static TabBandNotifyData* FromNmhdr(NMHDR* nmhdr) {
    #pragma warning(suppress: 4946)
    return reinterpret_cast<TabBandNotifyData*>(nmhdr);
  }
};

const char16 TabBand__TabDragMsgStr[] = L"Evita.TabBand.TabDrag";
void TabBand__Init(HINSTANCE);

#endif //!defined(INCLUDE_control_tabBand_h)
