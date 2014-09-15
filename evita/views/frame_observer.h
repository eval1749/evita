// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_frame_observer_h)
#define INCLUDE_evita_views_frame_observer_h

class Frame;

namespace views {

class FrameObserver {
  public: FrameObserver();
  public: virtual ~FrameObserver();

  public: virtual void DidActivateFrame(Frame* frame) = 0;

  DISALLOW_COPY_AND_ASSIGN(FrameObserver);
};

}   // views

#endif //!defined(INCLUDE_evita_views_frame_observer_h)
