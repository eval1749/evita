// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_SCROLL_BAR_OBSERVER_H_
#define EVITA_UI_CONTROLS_SCROLL_BAR_OBSERVER_H_

#include "base/basictypes.h"

namespace ui {

class ScrollBarObserver {
 public:
  virtual ~ScrollBarObserver();
  virtual void DidClickLineDown() = 0;
  virtual void DidClickLineUp() = 0;
  virtual void DidClickPageDown() = 0;
  virtual void DidClickPageUp() = 0;
  virtual void DidMoveThumb(int value) = 0;

 protected:
  ScrollBarObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(ScrollBarObserver);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_SCROLL_BAR_OBSERVER_H_
