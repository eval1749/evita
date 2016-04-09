// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/frames/frame_list.h"

#include "evita/editor/application.h"
#include "evita/frames/frame.h"

namespace views {

FrameList::FrameList() : active_frame_(nullptr) {}

FrameList::~FrameList() {}

void FrameList::AddFrame(Frame* frame) {
  frames_.insert(frame);
  frame->AddObserver(this);
}

Frame* FrameList::FindFrameByHwnd(HWND hwnd) const {
  for (auto frame : frames_) {
    if (frame->AssociatedHwnd() == hwnd)
      return frame;
  }
  return nullptr;
}

void FrameList::RemoveFrame(Frame* frame) {
  auto it = frames_.find(frame);
  if (it != frames_.end())
    frames_.erase(it);

  if (frames_.empty()) {
    active_frame_ = nullptr;
    editor::Application::instance()->Quit();
    return;
  }

  if (active_frame_ == frame) {
    active_frame_ = nullptr;
    for (auto candidate : frames_) {
      if (!active_frame_ ||
          active_frame_->active_tick() < candidate->active_tick()) {
        active_frame_ = candidate;
      }
    }
  }
}

// FrameObserver
void FrameList::DidActivateFrame(Frame* frame) {
  active_frame_ = frame;
}

}  // namespace views
