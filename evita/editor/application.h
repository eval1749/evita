// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_APPLICATION_H_
#define EVITA_EDITOR_APPLICATION_H_

#include <memory>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"

class IoManager;

namespace base {
class MessageLoop;
}

namespace domapi {
enum class ScriptHostState;
class ViewEventHandler;
}

namespace editor {
class DomLock;
}

namespace metrics {
class TimeScope;
}

namespace views {
class ViewDelegateImpl;
}

namespace editor {

class Scheduler;

class Application final : public common::Singleton<Application> {
  DECLARE_SINGLETON_CLASS(Application);

 public:
  ~Application();

  editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  Scheduler* scheduler() const { return scheduler_.get(); }
  const base::string16& title() const;
  const base::string16& version() const;
  domapi::ViewEventHandler* view_event_handler() const;

  bool CalledOnValidThread() const;
  void DidStartScriptHost(domapi::ScriptHostState state);
  IoManager* GetIoManager() const { return io_manager_.get(); }
  void Quit();
  void Run();

 private:
  Application();

  const base::char16* GetTitle() const;
  bool OnIdle(int hint);
  void DoIdle();

  std::unique_ptr<editor::DomLock> dom_lock_;
  std::unique_ptr<IoManager> io_manager_;
  bool is_quit_;
  std::unique_ptr<base::MessageLoop> message_loop_;
  std::unique_ptr<Scheduler> scheduler_;
  std::unique_ptr<metrics::TimeScope> view_idle_time_scope_;
  int view_idle_count_;
  int view_idle_hint_;
  std::unique_ptr<views::ViewDelegateImpl> view_delegate_impl_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace editor

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(::editor::Application::instance()->CalledOnValidThread())

#endif  // EVITA_EDITOR_APPLICATION_H_
