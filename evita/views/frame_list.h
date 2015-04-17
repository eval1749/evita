// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_frame_list_h)
#define INCLUDE_evita_views_frame_list_h

#include <windows.h>

#include <unordered_set>

#include "common/memory/singleton.h"
#include "evita/views/frame_observer.h"

class Frame;

namespace views {

class FrameList : public common::Singleton<FrameList>,
                  public FrameObserver {
  DECLARE_SINGLETON_CLASS(FrameList);

  private: Frame* active_frame_;
  private: std::unordered_set<Frame*> frames_;

  private: FrameList();
  public: virtual ~FrameList();

  public: Frame* active_frame() const { return active_frame_; }

  public: void AddFrame(Frame* frame);
  public: Frame* FindFrameByHwnd(HWND hwnd) const;
  public: void RemoveFrame(Frame* frame);

  // FrameObserver
  public: virtual void DidActivateFrame(Frame* frame) override;

  DISALLOW_COPY_AND_ASSIGN(FrameList);
};

}   // views

#endif //!defined(INCLUDE_evita_views_frame_list_h)
