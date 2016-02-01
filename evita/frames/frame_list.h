// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_FRAMES_FRAME_LIST_H_
#define EVITA_FRAMES_FRAME_LIST_H_

#include <windows.h>

#include <unordered_set>

#include "common/memory/singleton.h"
#include "evita/frames/frame_observer.h"

class Frame;

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FrameList
//
class FrameList : public common::Singleton<FrameList>, public FrameObserver {
  DECLARE_SINGLETON_CLASS(FrameList);

 public:
  ~FrameList() final;

  Frame* active_frame() const { return active_frame_; }

  void AddFrame(Frame* frame);
  Frame* FindFrameByHwnd(HWND hwnd) const;
  void RemoveFrame(Frame* frame);

 private:
  FrameList();

  // FrameObserver
  void DidActivateFrame(Frame* frame) final;

  Frame* active_frame_;
  std::unordered_set<Frame*> frames_;

  DISALLOW_COPY_AND_ASSIGN(FrameList);
};

}  // namespace views

#endif  // EVITA_FRAMES_FRAME_LIST_H_
