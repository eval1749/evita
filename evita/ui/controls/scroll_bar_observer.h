// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_scroll_bar_observer_h)
#define INCLUDE_evita_ui_controls_scroll_bar_observer_h

#include "base/basictypes.h"

namespace ui {

class ScrollBarObserver {
  protected: ScrollBarObserver();
  public: virtual ~ScrollBarObserver();

  public: virtual void DidClickLineDown() = 0;
  public: virtual void DidClickLineUp() = 0;
  public: virtual void DidClickPageDown() = 0;
  public: virtual void DidClickPageUp() = 0;
  public: virtual void DidMoveThumb(int value) = 0;

  DISALLOW_COPY_AND_ASSIGN(ScrollBarObserver);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_scroll_bar_observer_h)
