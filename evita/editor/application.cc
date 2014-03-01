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
//#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/dialog_box.h"
#include "evita/editor/dom_lock.h"
#include "evita/io/io_thread.h"
#include "evita/views/frame_list.h"
#include "evita/io/io_manager.h"
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
      view_delegate_impl_(new views::ViewDelegateImpl()) {
  io_manager_->Start();
  dom::ScriptThread::Start(message_loop_.get(), view_delegate_impl_.get(),
                           io_manager_->message_loop(),
                           io_manager_->io_delegate());
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

dom::ViewEventHandler* Application::view_event_handler() const {
  return view_delegate_impl_->event_handler();
}

bool Application::CalledOnValidThread() const {
  return message_loop_.get() == base::MessageLoop::current();
}

void Application::DoIdle() {
  #if DEBUG_IDLE
    DVLOG(0) << "idle_count_=" << idle_count_ <<
        " running=" << message_loop_->is_running() <<
        " modal=" << message_loop_->os_modal_loop();
  #endif
  if (!message_loop_->os_modal_loop() && TryDoIdle()) {
    ++idle_count_;
    message_loop_->PostTask(FROM_HERE, base::Bind(&Application::DoIdle,
                                                  base::Unretained(this)));
  } else {
    idle_count_= 0;
    message_loop_->PostNonNestableDelayedTask(FROM_HERE,
        base::Bind(&Application::DoIdle, base::Unretained(this)),
        base::TimeDelta::FromMilliseconds(1000 / 60));
  }
}

bool Application::OnIdle(uint hint) {
  return views::FrameList::instance()->DoIdle(static_cast<int>(hint));
}

void Application::Quit() {
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}

void Application::Run() {
  DoIdle();
  base::RunLoop run_loop;
  run_loop.Run();
}

// TryDoIdle() returns true if more works are needed.
bool Application::TryDoIdle() {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked())
    return true;
  if (!OnIdle(static_cast<uint>(idle_count_)))
    return false;
  auto const status = ::GetQueueStatus(QS_ALLEVENTS);
  #if DEBUG_IDLE
  if (status) {
    DVLOG(0) << "We have messages in queue, status=" << std::hex << status;
  }
  #endif
  return status;
}
