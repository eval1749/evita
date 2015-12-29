// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(eval1749): We should not include "windows.h" here.
#include <windows.h>

#include <commctrl.h>

#include "evita/editor/application.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application_proxy.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/scheduler.h"
#include "evita/editor/switch_set.h"
#include "evita/editor/trace_log_controller.h"
#include "evita/io/io_manager.h"
#include "evita/io/io_thread.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/paint/paint_thread.h"
#include "evita/spellchecker/spelling_engine.h"
#include "evita/ui/base/ime/text_input_client_win.h"
#include "evita/ui/widget.h"
#include "evita/views/forms/form_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/switches.h"
#include "evita/views/view_thread_proxy.h"
#include "evita/vi_Frame.h"

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
    : is_quit_(false),
      io_manager_(new io::IoManager()),
      message_loop_(new base::MessageLoop(base::MessageLoop::TYPE_UI)),
      paint_thread_(new paint::PaintThread()),
      trace_log_controller_(new TraceLogController()),
      view_delegate_(new views::ViewThreadProxy(message_loop_.get())),
      script_thread_(
          new dom::ScriptThread(view_delegate_.get(), io_manager_->proxy())),
      scheduler_(new Scheduler(script_thread_.get())) {
  io_manager_->Start();
  ui::TextInputClientWin::instance()->Start();
  paint_thread_->Start();
  script_thread_->Start();
  io_manager_->message_loop()->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(
          base::IgnoreResult(&spellchecker::SpellingEngine::EnsureInitialized),
          base::Unretained(spellchecker::SpellingEngine::GetSpellingEngine())));
}

Application::~Application() {}

const base::string16& Application::title() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, title, (APP_TITLE L" " APP_VERSION));
  return title;
}

const base::string16& Application::version() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, version, (APP_VERSION));
  return version;
}

domapi::ViewEventHandler* Application::view_event_handler() const {
  return script_thread_.get();
}

bool Application::CalledOnValidThread() const {
  return message_loop_.get() == base::MessageLoop::current();
}

void Application::DidStartScriptHost(domapi::ScriptHostState state) {
  if (state != domapi::ScriptHostState::Running) {
    // TODO(eval1749): We should set exit code other than EXIT_SUCCESS.
    editor::Application::instance()->Quit();
    return;
  }
  scheduler_->Start();
}

Application* Application::GetInstance() {
  return base::Singleton<Application>::get();
}

void Application::NotifyViewBusy() {
  auto const now = base::Time::Now();
  if (busy_start_time_ == base::Time()) {
    busy_start_time_ = now;
    return;
  }
  auto const delta = now - busy_start_time_;
  if (delta < base::TimeDelta::FromSeconds(1))
    return;
  auto const active_frame = views::FrameList::instance()->active_frame();
  if (!active_frame)
    return;
  auto const message = base::StringPrintf(
      L"Script runs %ds. Ctrl+Break to terminate script.", delta.InSeconds());
  active_frame->ShowMessage(MessageLevel_Warning, message);
}

void Application::NotifyViewReady() {
  busy_start_time_ = base::Time();
}

void Application::Quit() {
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}

void Application::Run() {
  {
    INITCOMMONCONTROLSEX init_params;
    init_params.dwSize = sizeof(init_params);
    init_params.dwICC = ICC_BAR_CLASSES;
    if (!::InitCommonControlsEx(&init_params)) {
      ::MessageBoxW(nullptr, L"InitCommonControlsEx",
                    APP_TITLE L" " APP_VERSION, MB_APPLMODAL | MB_ICONERROR);
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

  auto const command_line = base::CommandLine::ForCurrentProcess();
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
}

}  // namespace editor
