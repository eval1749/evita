// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_h)
#define INCLUDE_evita_views_tab_strip_h

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

#endif //!defined(INCLUDE_evita_views_tab_strip_h)
