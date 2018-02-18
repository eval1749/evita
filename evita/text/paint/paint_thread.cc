// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/paint/paint_thread.h"

#include <atomic>
#include <queue>
#include <utility>

#include "base/bind.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "evita/base/maybe.h"
#include "evita/editor/application.h"
#include "evita/gfx/dx_device.h"
#include "evita/ui/compositor/compositor.h"

namespace paint {

namespace {

//////////////////////////////////////////////////////////////////////
//
// TaskQueue
//
class TaskQueue final {
 public:
  TaskQueue() = default;
  ~TaskQueue() = default;

  bool empty() const { return tasks_.empty(); }
  size_t size() const { return tasks_.size(); }

  void GiveTask(base::OnceClosure closure);
  base::Maybe<base::OnceClosure> TakeTask();

 private:
  base::Lock lock_;
  std::queue<base::OnceClosure> tasks_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueue);
};

void TaskQueue::GiveTask(base::OnceClosure task) {
  base::AutoLock lock_scope(lock_);
  tasks_.push(std::move(task));
}

base::Maybe<base::OnceClosure> TaskQueue::TakeTask() {
  base::AutoLock lock_scope(lock_);
  if (tasks_.empty())
    return base::Nothing<base::OnceClosure>();
  auto task = std::move(tasks_.front());
  tasks_.pop();
  return std::move(base::Just(std::move(task)));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PaintThread::Scheduler
//
class PaintThread::Scheduler final {
 public:
  explicit Scheduler(PaintThread* paint_thread);
  ~Scheduler() = default;

  void ScheduleTask(base::OnceClosure task);

 private:
  enum class State {
    Running,
    Sleeping,
  };

  void ProcessTasks();

  PaintThread* const paint_thread_;
  std::atomic<State> state_;
  TaskQueue task_queue_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

PaintThread::Scheduler::Scheduler(PaintThread* paint_thread)
    : paint_thread_(paint_thread), state_(State::Sleeping) {}

void PaintThread::Scheduler::ProcessTasks() {
  state_.store(State::Running);
  for (;;) {
    auto maybe_task = task_queue_.TakeTask();
    if (maybe_task.IsNothing())
      break;
    std::move(maybe_task.MoveFromJust()).Run();
  }
  state_.store(State::Sleeping);
  paint_thread_->CommitIfNeeded();
}

void PaintThread::Scheduler::ScheduleTask(base::OnceClosure task) {
  task_queue_.GiveTask(std::move(task));
  if (state_.load() == State::Running)
    return;
  paint_thread_->PostTask(FROM_HERE, base::BindOnce(&Scheduler::ProcessTasks,
                                                    base::Unretained(this)));
}

#define DCHECK_CALLED_ON_NON_PAINT_THREAD() \
  DCHECK_NE(thread_->message_loop(), base::MessageLoop::current())

#define DCHECK_CALLED_ON_PAINT_THREAD() \
  DCHECK_EQ(thread_->message_loop(), base::MessageLoop::current())

//////////////////////////////////////////////////////////////////////
//
// PaintThread
//
PaintThread::PaintThread()
    : scheduler_(new Scheduler(this)),
      thread_(new base::Thread("paint thread")) {}

PaintThread::~PaintThread() {}

ui::Compositor* PaintThread::compositor() const {
  DCHECK_CALLED_ON_PAINT_THREAD();
  DCHECK(compositor_);
  return compositor_.get();
}

gfx::DxDevice* PaintThread::device() const {
  DCHECK_CALLED_ON_PAINT_THREAD();
  DCHECK(device_);
  return device_.get();
}

// static
PaintThread* PaintThread::instance() {
  return editor::Application::instance()->paint_thread();
}

void PaintThread::CommitIfNeeded() {
  DCHECK_CALLED_ON_PAINT_THREAD();
  compositor_->CommitIfNeeded();
}

void PaintThread::DidStartThread() {
  DCHECK_CALLED_ON_PAINT_THREAD();
  device_.reset(new gfx::DxDevice());
  compositor_.reset(new ui::Compositor(device_.get()));
}

void PaintThread::PostTask(const base::Location& from_here,
                           base::OnceClosure task) {
  DCHECK_CALLED_ON_NON_PAINT_THREAD();
  thread_->task_runner()->PostTask(from_here, std::move(task));
}

void PaintThread::SchedulePaintTask(base::OnceClosure task) {
  DCHECK_CALLED_ON_NON_PAINT_THREAD();
  scheduler_->ScheduleTask(std::move(task));
}

void PaintThread::Start() {
  DCHECK_CALLED_ON_NON_PAINT_THREAD();
  CHECK(thread_->Start()) << "failed to start paint thread";
  PostTask(FROM_HERE, base::BindOnce(&PaintThread::DidStartThread,
                                     base::Unretained(this)));
}

}  // namespace paint
