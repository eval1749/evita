// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/demo/demo_window.h"

#include "base/time/time.h"
#include "base/message_loop/message_loop.h"
#include "build/build_config.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
DemoWindow::DemoWindow(const base::Closure& quit_closure)
    : ui::AnimatableWindow(ui::NativeWindow::Create(this)),
      quit_closure_(quit_closure) {}

DemoWindow::~DemoWindow() {
  base::MessageLoop::current()->task_runner()->PostTask(FROM_HERE,
                                                        quit_closure_);
}

// ui::AnimationFrameHandler
void DemoWindow::DidBeginAnimationFrame(base::Time time) {}

}  // namespace visuals
