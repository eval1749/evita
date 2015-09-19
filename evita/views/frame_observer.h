// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FRAME_OBSERVER_H_
#define EVITA_VIEWS_FRAME_OBSERVER_H_

#include "base/macros.h"

class Frame;

namespace views {

class FrameObserver {
 public:
  virtual ~FrameObserver();

  virtual void DidActivateFrame(Frame* frame) = 0;

 protected:
  FrameObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(FrameObserver);
};

}  // namespace views

#endif  // EVITA_VIEWS_FRAME_OBSERVER_H_
