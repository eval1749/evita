// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <unordered_set>

#include "evita/visuals/demo/demo_application.h"

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/process/launch.h"
#include "base/run_loop.h"
#include "evita/ui/animation/animation_scheduler.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/visuals/demo/demo_model.h"
#include "evita/visuals/demo/demo_window.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// DemoScheduler
//
class DemoScheduler : public ui::AnimationScheduler {
 public:
  DemoScheduler() { ui::AnimationScheduler::GetInstance()->SetScheduler(this); }

  ~DemoScheduler() = default;

 private:
  void BeginFrame();

  // ui::AnimationScheduler
  void CancelAnimationFrameRequest(ui::AnimationFrameHandler* handler) final;
  void RequestAnimationFrame(ui::AnimationFrameHandler* handler) final;

  bool is_waiting_ = false;
  base::Time last_frame_time_;
  std::unordered_set<ui::AnimationFrameHandler*> pending_handlers_;

  DISALLOW_COPY_AND_ASSIGN(DemoScheduler);
};

void DemoScheduler::BeginFrame() {
  std::unordered_set<ui::AnimationFrameHandler*> ready_handlers;
  ready_handlers.swap(pending_handlers_);
  const auto& now = base::Time::Now();
  last_frame_time_ = now;
  for (const auto& handler : ready_handlers)
    handler->HandleAnimationFrame(now);
  ui::Compositor::instance()->CommitIfNeeded();
}

// ui::AnimationScheduler
void DemoScheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  pending_handlers_.erase(handler);
}

void DemoScheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  pending_handlers_.insert(handler);
  if (is_waiting_)
    return;
  const auto next_frame_time =
      last_frame_time_ + base::TimeDelta::FromMilliseconds(1000 / 60);
  const auto delta = next_frame_time - base::Time::Now();
  base::MessageLoop::current()->task_runner()->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&DemoScheduler::BeginFrame, base::Unretained(this)),
      std::min(base::TimeDelta::FromMilliseconds(3),
               std::max(delta, base::TimeDelta::FromMilliseconds(3))));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DemoApplication
//
DemoApplication::DemoApplication() : model_(new DemoModel()) {}
DemoApplication::~DemoApplication() {}

void DemoApplication::Run() {
  base::AtExitManager at_exit;
  DemoScheduler scheduler;
  base::RouteStdioToConsole(true);
  auto message_loop =
      std::make_unique<base::MessageLoop>(base::MessageLoop::TYPE_UI);
  base::RunLoop run_loop;
  const auto& window = new DemoWindow(run_loop.QuitClosure());
  window->RealizeWidget();
  model_->AttachWindow(window);
  run_loop.Run();
}

}  // namespace visuals
