// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_application_h)
#define INCLUDE_evita_editor_application_h

#include <memory>

// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible

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

namespace ui {
class AnimationFrameHandler;
class AnimationScheduler;
}

namespace views {
class ViewDelegateImpl;
}

class Application : public common::Singleton<Application> {
  DECLARE_SINGLETON_CLASS(Application);

  private: std::unique_ptr<editor::DomLock> dom_lock_;
  private: std::unique_ptr<IoManager> io_manager_;
  private: bool is_quit_;
  private: std::unique_ptr<base::MessageLoop> message_loop_;
  private: std::unique_ptr<ui::AnimationScheduler> animation_scheduler_;
  private: std::unique_ptr<metrics::TimeScope> view_idle_time_scope_;
  private: int view_idle_count_;
  private: int view_idle_hint_;
  private: std::unique_ptr<views::ViewDelegateImpl> view_delegate_impl_;

  private: Application();
  public: ~Application();

  public: ui::AnimationScheduler* animation_scheduler() const {
    return animation_scheduler_.get();
  }
  public: editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  public: const base::string16& title() const;
  public: const base::string16& version() const;
  public: domapi::ViewEventHandler* view_event_handler() const;

  // [C]
  public: bool CalledOnValidThread() const;

  // [D]
  public: void DidStartScriptHost(domapi::ScriptHostState state);
  public: void DidHandleViewIdelEvent(int hint);
  private: void DispatchViewIdelEvent();
  private: void DoIdle();

  // [G]
  public: IoManager* GetIoManager() const { return io_manager_.get(); }
  private: const base::char16* GetTitle() const;

  // [O]
  private: bool OnIdle(int hint);

  // [Q]
  public: void Quit();

  // [R]
  public: void Run();

  DISALLOW_COPY_AND_ASSIGN(Application);
};

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(Application::instance()->CalledOnValidThread())


#endif //!defined(INCLUDE_evita_editor_application_h)
