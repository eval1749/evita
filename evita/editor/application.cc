// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/application.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_observer.h"
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application_proxy.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/switch_set.h"
#include "evita/gfx/dx_device.h"
#include "evita/io/io_manager.h"
#include "evita/io/io_thread.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/spellchecker/spelling_engine.h"
#include "evita/ui/animation/animation_scheduler.h"
#include "evita/ui/base/ime/text_input_client_win.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/widget.h"
#include "evita/views/frame_list.h"
#include "evita/views/forms/form_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/switches.h"
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

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// Application
//
Application::Application()
    : dom_lock_(new editor::DomLock()),
      is_quit_(false),
      io_manager_(new IoManager()),
      message_loop_(new base::MessageLoop(base::MessageLoop::TYPE_UI)),
      animation_scheduler_(new ui::AnimationScheduler(message_loop_.get())),
      view_idle_count_(0),
      view_idle_hint_(0),
      view_delegate_impl_(new views::ViewDelegateImpl()) {
  io_manager_->Start();
  ui::TextInputClientWin::instance()->Start();
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

void Application::DidHandleViewIdelEvent(int) {
  view_idle_time_scope_.reset();
  view_idle_hint_ = view_idle_count_;
  view_idle_count_ = 0;
}

void Application::DidStartScriptHost(domapi::ScriptHostState state) {
  if (state != domapi::ScriptHostState::Running) {
    // TODO(yosi) We should set exit code other than EXIT_SUCCESS.
    editor::Application::instance()->Quit();
    return;
  }
  DispatchViewIdelEvent();
}

void Application::DispatchViewIdelEvent() {
  #if DEBUG_IDLE
    DVLOG(0) << "view_idle_count_=" << view_idle_count_ << " active_focus=" <<
            ui::FocusController::instance()->has_active_focus();
  #endif
  if (view_idle_count_) {
    // DOM is still processing "view idle" vent.
    METRICS_COUNT("view_idle_count");
    ++view_idle_count_;
  } else if (ui::FocusController::instance()->has_active_focus()) {
    // We are active. Notify DOM to do something.
    METRICS_COUNT("view_idle_event");
    view_idle_count_ = 1;
    view_idle_time_scope_.reset(new ::metrics::TimeScope(
        "Application::DispatchViewIdelEvent"));
    view_event_handler()->DispatchViewIdleEvent(view_idle_hint_);
  } else {
    // We are in active.
    METRICS_COUNT("view_idle_hint");
    ++view_idle_hint_;
  }
  message_loop_->PostNonNestableDelayedTask(FROM_HERE,
      base::Bind(&Application::DispatchViewIdelEvent, base::Unretained(this)),
      base::TimeDelta::FromMilliseconds(100));
}

void Application::Quit() {
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}

void Application::Run() {
  {
      INITCOMMONCONTROLSEX init_params;
      init_params.dwSize = sizeof(init_params);
      init_params.dwICC  = ICC_BAR_CLASSES;
      if (!::InitCommonControlsEx(&init_params)) {
          ::MessageBoxW(
              nullptr,
              L"InitCommonControlsEx",
              APP_TITLE L" " APP_VERSION,
              MB_APPLMODAL | MB_ICONERROR);
          return;
      }
  }

  #if _DEBUG
  views::switches::editor_window_display_paint = true;
  views::switches::form_window_display_paint = false;
  views::switches::text_window_display_paint = true;
  #endif

  auto const switch_set = editor::SwitchSet::instance();

  switch_set->Register(views::switches::kEditorWindowDisplayPaint,
                       &views::switches::editor_window_display_paint);
  switch_set->Register(views::switches::kFormWindowDisplayPaint,
                       &views::switches::form_window_display_paint);
  switch_set->Register(views::switches::kTextWindowDisplayPaint,
                       &views::switches::text_window_display_paint);

  auto const command_line = CommandLine::ForCurrentProcess();
  for (const auto& name : switch_set->names()) {
    const auto value = switch_set->Get(name);
    if (value.is_bool()) {
      if (command_line->HasSwitch(base::UTF16ToASCII(name)))
        switch_set->Set(name, domapi::SwitchValue(true));
    }
  }

  ApplicationProxy::instance()->WillStartApplication();
  base::RunLoop run_loop;
  run_loop.Run();

  // TODO(eval1749) We should use Singleton destructor for destructing
  // |gfx::DxDevice|.
  delete gfx::DxDevice::instance();
  // TODO(eval1749) We should use Singleton destructor for destructing
  // |ui::Compositor|.
  delete ui::Compositor::instance();
}

}  // namespace editor
