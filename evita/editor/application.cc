// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/application.h"

#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable: 4100)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "common/memory/scoped_change.h"
#include "evita/cm_CmdProc.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/dialog_box.h"
#include "evita/editor/dom_lock.h"
#include "evita/ed_Mode.h"
#include "evita/dom/buffer.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_FileDialogBox.h"
#include "evita/vi_IoManager.h"
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

Application::Application()
    : newline_mode_(NewlineMode_CrLf),
      code_page_(932),
      active_frame_(nullptr),
      idle_count_(0),
      is_quit_(false),
      command_processor_(new Command::Processor()),
      dom_lock_(new editor::DomLock()),
      io_manager_(new IoManager()),
      message_loop_(new base::MessageLoop(base::MessageLoop::TYPE_UI)),
      view_delegate_impl_(new views::ViewDelegateImpl()) {
  Command::Processor::GlobalInit();
  io_manager_->Realize();
  dom::ScriptThread::Start(view_delegate_impl_.get(), message_loop_.get());
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
  ::LoadString(g_hResource, format_id, wszFormat, arraysize(wszFormat));

  char16 wsz[1024];
  va_list args;
  va_start(args, format_id);
  ::wvsprintf(wsz, wszFormat, args);
  va_end(args);

  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  return ::MessageBoxW(*GetActiveFrame(), wsz, title().c_str(), flags);
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

void Application::Execute(CommandWindow* window, uint32 key_code,
                          uint32 repeat) {
  command_processor_->Execute(window, key_code, repeat);
}

Buffer* Application::FindBuffer(const base::string16& name) const {
  for (auto& buffer: buffers_) {
    if (buffer.name() == name)
      return const_cast<Buffer*>(&buffer);
  }
  return nullptr;
}

Frame* Application::FindFrame(HWND hwnd) const {
  for (auto& frame: frames_) {
      if (frame == hwnd)
          return const_cast<Frame*>(&frame);
  }
  return nullptr;
}

Pane* Application::FindPane(HWND hwndMouse, POINT pt) const {
  if (!::ClientToScreen(hwndMouse, &pt))
    return nullptr;
  auto hwnd = ::WindowFromPoint(pt);
  if (!hwnd)
    return nullptr;

  if (!g_nDropTargetMsg) {
    static const char16 Evita__DropTarget[] = L"Evita.DropTarget";
    g_nDropTargetMsg = ::RegisterWindowMessage(Evita__DropTarget);
    if (!g_nDropTargetMsg)
      return nullptr;
  }

  do {
    if (auto const iAnswer = ::SendMessage(hwnd, g_nDropTargetMsg, 0, 0))
      return reinterpret_cast<Pane*>(iAnswer);
    hwnd = ::GetParent(hwnd);
  } while (hwnd);

  return nullptr;
}

HIMAGELIST Application::GetIconList() const {
  return text::ModeFactory::icon_image_list();
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

namespace {
typedef std::pair<base::string16, base::string16> StringPair;
StringPair SplitByDot(const base::string16& name) {
  const auto last_dot = name.rfind('.');
  if (!last_dot || last_dot == base::string16::npos)
    return StringPair(name, L"");
  return StringPair(name.substr(0, last_dot), name.substr(last_dot));
}
}  // namespace

Buffer* Application::RenameBuffer(Buffer* buffer,
                                  const base::string16& new_name) {
  auto const present = FindBuffer(new_name);
  if (buffer == present)
    return buffer;

  if (!present) {
    buffer->SetName(new_name);
    return buffer;
  }

  const auto pair = SplitByDot(new_name);
  auto candidate = new_name;
  for (auto n = 2; FindBuffer(candidate); ++ n) {
    candidate = pair.first + L" (" + base::IntToString16(n) + L")" +
        pair.second;
  }

  buffer->SetName(candidate);
  return buffer;
}

void Application::Run() {
  DoIdle();
  base::RunLoop run_loop;
  run_loop.Run();
}

bool Application::SaveBuffer(Frame* frame, Buffer* buffer, bool is_save_as) {
  auto eNewline = buffer->GetNewline();
  if (NewlineMode_Detect == eNewline)
    eNewline = GetNewline();

  auto code_page = buffer->GetCodePage();
  if (!code_page)
    code_page = GetCodePage();

  FileDialogBox::Param oParam;
  oParam.m_hwndOwner = *frame;

  oParam.m_wsz[0] = 0;
  if (is_save_as || !buffer->GetFileName()[0]) {
    ::lstrcpyW(oParam.m_wsz, buffer->GetFileName().c_str());
    FileDialogBox oDialog;
    if (!oDialog.GetSaveFileName(&oParam))
      return true;

    auto pwszName = ::lstrrchrW(oParam.m_wsz, '\\');
    if (!pwszName) {
       pwszName = ::lstrrchrW(oParam.m_wsz, '/');
       if (!pwszName)
         pwszName = oParam.m_wsz + 1;
    }

    // Skip slash(/)
    ++pwszName;
    RenameBuffer(buffer, pwszName);
  }

  return buffer->Save(oParam.m_wsz, code_page, eNewline);
}

void Application::ShowMessage(MessageLevel iLevel, uint nFormatId) {
  GetActiveFrame()->ShowMessage(iLevel, nFormatId);
}

// TryDoIdle() returns true if more works are needed.
bool Application::TryDoIdle() {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(dom_lock);
  if (!dom_lock.locked())
    return true;
  return OnIdle(idle_count_);
}

void Application::WillDestroyFrame(Frame* frame) {
  frames_.Delete(frame);
  if (!frames_.IsEmpty())
    return;
  is_quit_ = true;
  message_loop_->QuitWhenIdle();
}
