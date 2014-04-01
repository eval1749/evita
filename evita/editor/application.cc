// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/application.h"

#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_observer.h"
#include "base/run_loop.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/dom_lock.h"
#include "evita/io/io_manager.h"
#include "evita/io/io_thread.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/spellchecker/spelling_engine.h"
#include "evita/ui/widget.h"
#include "evita/views/frame_list.h"
#include "evita/views/forms/form_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/view_delegate_impl.h"

#define DEBUG_IDLE 0

#if _DEBUG
  #define APP_TITLE L"evita/debug"
#else
  #define APP_TITLE L"evita"
#endif
#define APP_VERSION L"5.0"


HINSTANCE g_hInstance;
HINSTANCE g_hResource;
HWND g_hwndActiveDialog;

namespace {
class MessagePump : public base::MessagePumpForUI {
  private: class MessageFilter : public base::MessagePumpForUI::MessageFilter {
    public: MessageFilter() = default;
    public: ~MessageFilter() = default;

    // base::MessagePumpForUI:::MessageFilter
    private: virtual bool ProcessMessage(const MSG& msg) {
      if (!g_hwndActiveDialog)
        return false;
      MSG message = msg;
      return ::IsDialogMessage(g_hwndActiveDialog, &message);
    }
    DISALLOW_COPY_AND_ASSIGN(MessageFilter);
  };

  public: MessagePump() {
    SetMessageFilter(make_scoped_ptr(new MessageFilter()));
  }
  public: ~MessagePump() = default;

  DISALLOW_COPY_AND_ASSIGN(MessagePump);
};
}   // namespace

//////////////////////////////////////////////////////////////////////
//
// Application
//
Application::Application()
    : idle_count_(0),
            is_quit_(false),
      dom_lock_(new editor::DomLock()),
      io_manager_(new IoManager()),
      message_loop_(new base::MessageLoop(make_scoped_ptr(new MessagePump()))),
      view_idle_count_(0),
      view_idle_hint_(0),
      view_delegate_impl_(new views::ViewDelegateImpl()) {
  io_manager_->Start();
  dom::ScriptThread::Start(message_loop_.get(), view_delegate_impl_.get(),
                           io_manager_->message_loop(),
                           io_manager_->io_delegate());
  io_manager_->message_loop()->PostTask(FROM_HERE, base::Bind(
      base::IgnoreResult(&spellchecker::SpellingEngine::EnsureInitialized),
      base::Unretained(spellchecker::SpellingEngine::GetSpellingEngine())));
}

Application::~Application() {
}

const base::string16& Application::title() const {
  DEFINE_STATIC_LOCAL(base::string16, title, (APP_TITLE L" " APP_VERSION));
  return title;
}

const base::string16& Application::version() const {
  DEFINE_STATIC_LOCAL(base::string16, version, (APP_VERSION));
  return version;
}

domapi::ViewEventHandler* Application::view_event_handler() const {
  return view_delegate_impl_->event_handler();
}

bool Application::CalledOnValidThread() const {
  return message_loop_.get() == base::MessageLoop::current();
}

void Application::DidStartScriptHost(domapi::ScriptHostState state) {
  if (state != domapi::ScriptHostState::Running) {
    // TODO(yosi) We should set exit code other than EXIT_SUCCESS.
    Application::instance()->Quit();
    return;
  }
  DoIdle();
  DispatchViewIdelEvent();
}

void Application::DidHandleViewIdelEvent(int) {
  view_idle_hint_ = view_idle_count_;
  view_idle_count_ = 0;
}

void Application::DispatchViewIdelEvent() {
  #if DEBUG_IDLE
    DVLOG(0) << "view_idle_count_=" << view_idle_count_ << " active_focus=" <<
            ui::Widget::has_active_focus();
  #endif
  if (view_idle_count_) {
    ++view_idle_count_;
  } if (ui::Widget::has_active_focus()) {
    view_event_handler()->DispatchViewIdleEvent(view_idle_hint_);
    view_idle_count_ = 1;
  } else {
    ++view_idle_hint_;
  }
  message_loop_->PostNonNestableDelayedTask(FROM_HERE,
      base::Bind(&Application::DispatchViewIdelEvent, base::Unretained(this)),
      base::TimeDelta::FromMilliseconds(100));
}

// Note: We don't need to check ::GetQueueStatus (QS_INPUT), becaue |DoIdle()|
// is called after processing Windows message.
void Application::DoIdle() {
  base::TimeDelta wait_time;
  if (message_loop_->os_modal_loop()) {
    METRICS_COUNT("os_modal_loop");
    wait_time = base::TimeDelta::FromMilliseconds(100);
  } else if (OnIdle(idle_count_)) {
    METRICS_COUNT("OnIdle");
    ++idle_count_;
    wait_time = base::TimeDelta::FromMilliseconds(1000 / 60);
  } else {
    METRICS_COUNT("nothing");
    // Nothing to do.
    idle_count_= 0;
    wait_time = base::TimeDelta::FromMilliseconds(1000 / 60);
  }
  message_loop_->PostNonNestableDelayedTask(FROM_HERE,
      base::Bind(&Application::DoIdle, base::Unretained(this)),
      wait_time);
}

bool Application::OnIdle(int hint) {
  METRICS_TIME_SCOPE();
  auto more = views::FrameList::instance()->DoIdle(hint);
  more |= views::FormWindow::DoIdle(hint);
  return more;
}

void Application::Quit() {
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}

void Application::Run() {
  base::RunLoop run_loop;
  run_loop.Run();
}
