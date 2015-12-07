// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_APPLICATION_H_
#define EVITA_EDITOR_APPLICATION_H_

#include <memory>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"

namespace base {
class MessageLoop;
}

namespace dom {
class ScriptThread;
}

namespace domapi {
enum class ScriptHostState;
class ViewEventHandler;
}

namespace editor {
class DomLock;
}

namespace io {
class IoManager;
}

namespace metrics {
class TimeScope;
}

namespace paint {
class PaintThread;
}

namespace views {
class ViewThreadProxy;
}

namespace editor {

class Scheduler;
class TraceLogController;

class Application final : public common::Singleton<Application> {
  DECLARE_SINGLETON_CLASS(Application);

 public:
  editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  io::IoManager* io_manager() const { return io_manager_.get(); }
  paint::PaintThread* paint_thread() const { return paint_thread_.get(); }
  Scheduler* scheduler() const { return scheduler_.get(); }
  const base::string16& title() const;
  TraceLogController* trace_log_controller() const {
    return trace_log_controller_.get();
  }
  const base::string16& version() const;
  domapi::ViewEventHandler* view_event_handler() const;

  bool CalledOnValidThread() const;
  void DidStartScriptHost(domapi::ScriptHostState state);
  void NotifyViewBusy();
  void NotifyViewReady();
  void Quit();
  void Run();

 private:
  Application();
  ~Application() final;

  std::unique_ptr<editor::DomLock> dom_lock_;
  const std::unique_ptr<io::IoManager> io_manager_;
  bool is_quit_;
  const std::unique_ptr<base::MessageLoop> message_loop_;
  const std::unique_ptr<paint::PaintThread> paint_thread_;
  const std::unique_ptr<TraceLogController> trace_log_controller_;
  const std::unique_ptr<views::ViewThreadProxy> view_delegate_;

  // |dom::ScriptThread| uses |IoDelegate| and |ViewDelegate|.
  const std::unique_ptr<dom::ScriptThread> script_thread_;
  base::Time busy_start_time_;

  // |Scheduler| uses |domapi::ViewEventHandler|.
  const std::unique_ptr<Scheduler> scheduler_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace editor

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(::editor::Application::instance()->CalledOnValidThread())

#endif  // EVITA_EDITOR_APPLICATION_H_
