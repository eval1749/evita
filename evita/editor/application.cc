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
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "common/memory/scoped_change.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/dialog_box.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/buffer.h"
#include "evita/io/io_thread.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_FileDialogBox.h"
#include "evita/io/io_manager.h"
#include "evita/vi_TextEditWindow.h"
#include "evita/views/view_delegate_impl.h"
#include "evita/views/window_set.h"

#define DEBUG_IDLE 0

#if _DEBUG
  #define APP_TITLE L"evita/debug"
#else
  #define APP_TITLE L"evita"
#endif
#define APP_VERSION L"5.0"


UINT g_nDropTargetMsg;

HINSTANCE g_hInstance;
HINSTANCE g_hResource;
HWND g_hwndActiveDialog;
UINT g_TabBand__TabDragMsg;

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
    : active_frame_(nullptr),
      idle_count_(0),
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

int Application::Ask(int flags, int format_id, ...) {
  char16 wszFormat[1024];
  ::LoadString(g_hResource, static_cast<UINT>(format_id), wszFormat,
               arraysize(wszFormat));

  char16 wsz[1024];
  va_list args;
  va_start(args, format_id);
  ::wvsprintf(wsz, wszFormat, args);
  va_end(args);

  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  return ::MessageBoxW(*GetActiveFrame(), wsz, title().c_str(),
                       static_cast<UINT>(flags));
}

bool Application::CalledOnValidThread() const {
  return message_loop_.get() == base::MessageLoop::current();
}

void Application::DidCreateFrame(Frame* frame) {
  frames_.Append(frame);
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

Frame* Application::FindFrame(HWND hwnd) const {
  for (auto& frame: frames_) {
      if (frame == hwnd)
          return const_cast<Frame*>(&frame);
  }
  return nullptr;
}

bool Application::OnIdle(uint nCount) {
  auto need_more = false;
  for (auto& frame: frames_) {
    if (frame.OnIdle(nCount))
      need_more = true;
  }
  return need_more;
}

static void RunTaskWithinDomLock(const base::Closure& task) {
  UI_DOM_AUTO_LOCK_SCOPE();
  task.Run();
}

void Application::PostDomTask(const tracked_objects::Location& from_here,
                              const base::Closure& task) {
  message_loop_->PostTask(from_here, base::Bind(RunTaskWithinDomLock, task));
}

void Application::Run() {
  DoIdle();
  base::RunLoop run_loop;
  run_loop.Run();
}

void Application::ShowMessage(MessageLevel iLevel, uint nFormatId) {
  GetActiveFrame()->ShowMessage(iLevel, nFormatId);
}

// TryDoIdle() returns true if more works are needed.
bool Application::TryDoIdle() {
  UI_DOM_AUTO_LOCK_SCOPE();
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

void Application::WillDestroyFrame(Frame* frame) {
  frames_.Delete(frame);
  if (!frames_.IsEmpty())
    return;
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}
