// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/win/scoped_handle.h"
#include "common/win/singleton_hwnd.h"
#include "common/win/win32_verify.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/application_proxy.h"

namespace editor {

namespace {
const base::char16 SINGLE_INSTANCE_NAME[] =
    L"Local\\D47A7677-9F8E-467c-BABE-8ABDE8D58476";

const base::char16 k_wszFileMapping[] =
    L"Local\\03002DEC-D63E-4551-9AE8-B88E8C586376";

void __declspec(noreturn) FatalExit(const base::char16* message) {
  base::char16 wsz[100];
  ::wsprintf(wsz, L"Evita Text Editor can't start (%s).", message);
  ::FatalAppExit(0, wsz);
}

bool IsIndependentApplication() {
  auto const command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch("multiple");
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy::Channel
//
class ApplicationProxy::Channel {
  private: struct SharedArea {
    HWND m_hwnd;
    base::char16 m_wsz[1];
  };

  public: class ScopedMap {
    private: SharedArea* location_;

    public: ScopedMap(Channel* channel);
    public: ~ScopedMap();

    public: explicit operator bool() const { return location_ != nullptr; }
    public: SharedArea* operator->() const { return location_; }
  };
  friend class ScopedMap;

  private: static const DWORD k_cbFileMapping = 1024 * 64;

  private: common::win::scoped_handle handle_;

  private: Channel(HANDLE handle);
  public: ~Channel();

  public: operator bool() const { return handle_.is_valid(); }
  public: operator HANDLE() const { return handle_.get(); }

  public: static Channel* CreateChannel(HWND hwnd);
  public: static Channel* OpenChannel();

  DISALLOW_COPY_AND_ASSIGN(Channel);
};

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy::Channel::ScopedMap
//
ApplicationProxy::Channel::ScopedMap::ScopedMap(Channel* channel)
    : location_(reinterpret_cast<SharedArea*>(
        ::MapViewOfFile(*channel, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,
            k_cbFileMapping))) {
}

ApplicationProxy::Channel::ScopedMap::~ScopedMap() {
  if (!location_)
    return;
  WIN32_VERIFY(::UnmapViewOfFile(location_));
}

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy::Channel
//
ApplicationProxy::Channel::Channel(HANDLE handle) : handle_(handle) {
}

ApplicationProxy::Channel::~Channel() {
}

ApplicationProxy::Channel* ApplicationProxy::Channel::CreateChannel(HWND hwnd) {
  common::win::scoped_handle handle(::CreateFileMapping(
      INVALID_HANDLE_VALUE, // hFile
      nullptr, // lpAttributes
      PAGE_READWRITE, // flProtect
      0, // dwMaximumSizeHigh
      k_cbFileMapping, // dwMaximumSizeLow
      k_wszFileMapping)); // lpName

  if (!handle) {
    CHECK(handle) << "CreateFileMapping " << ::GetLastError();
    return nullptr;
  }

  std::unique_ptr<Channel> channel(new Channel(handle.release()));
  Channel::ScopedMap payload(channel.get());
  if (!payload)
    return nullptr;
  payload->m_hwnd = hwnd;
  return channel.release();
}

ApplicationProxy::Channel* ApplicationProxy::Channel::OpenChannel() {
  common::win::scoped_handle handle(::OpenFileMapping(
      FILE_MAP_READ | FILE_MAP_WRITE,
      FALSE,
      k_wszFileMapping));
  if (!handle)
      FatalExit(L"OpenFileMapping");
  return new Channel(handle.release());
}

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy::EventObject
//
class ApplicationProxy::EventObject {
  private: HANDLE handle_;
  private: bool is_existing_;

  public: EventObject(const base::char16* name);
  public: ~EventObject();

  public: explicit operator bool() const { return handle_ != nullptr; }

  public: bool is_existing() const { return is_existing_; }

  public: void Set();
  public: bool Wait(int wait_ms);
};

ApplicationProxy::EventObject::EventObject(const base::char16* name)
    : handle_(::CreateEventW(nullptr, true, false, name)),
      is_existing_(::GetLastError() == ERROR_ALREADY_EXISTS) {
}

ApplicationProxy::EventObject::~EventObject() {
  if (!handle_)
    return;
  ::CloseHandle(handle_);
}

void ApplicationProxy::EventObject::Set() {
  DCHECK(handle_);
  ::SetEvent(handle_);
}

bool ApplicationProxy::EventObject::Wait(int wait_ms) {
  DCHECK(handle_);
  auto const result = ::WaitForSingleObject(handle_,
                                            static_cast<DWORD>(wait_ms));
  if (result == WAIT_OBJECT_0)
    return true;
  DVLOG(ERROR) << "WaitForSingleObject " << ::GetLastError();
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy::ShellHandler
//
class ApplicationProxy::ShellHandler
    : public common::win::SingletonHwnd::Observer {
  public: ShellHandler();
  public: virtual ~ShellHandler();

  public: void Start();

  // common::win::SingletonHwnd::Observer
  private: virtual void OnWndProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam) override;

  DISALLOW_COPY_AND_ASSIGN(ShellHandler);
};

//////////////////////////////////////////////////////////////////////
//
// ShellHandler
//
ApplicationProxy::ShellHandler::ShellHandler() {
}

ApplicationProxy::ShellHandler::~ShellHandler() {
}

void ApplicationProxy::ShellHandler::Start() {
  common::win::SingletonHwnd::instance()->AddObserver(this);
}

// common::win::SingletonHwnd::Observer
void ApplicationProxy::ShellHandler::OnWndProc(HWND hwnd, UINT message, WPARAM,
                                               LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      ApplicationProxy::instance()->DidStartChannel(hwnd);
      return;

    case WM_COPYDATA:
      ApplicationProxy::instance()->DidCopyData(
          reinterpret_cast<COPYDATASTRUCT*>(lParam));
      return;
  }
}

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy
//
ApplicationProxy::ApplicationProxy()
    : event_(IsIndependentApplication()
        ? nullptr : new EventObject(SINGLE_INSTANCE_NAME)),
      shell_handler_(event_ ? new ShellHandler() : nullptr) {
}

ApplicationProxy::~ApplicationProxy() {
}

void ApplicationProxy::DidCopyData(const COPYDATASTRUCT* data) {
  auto const file_name = reinterpret_cast<base::char16*>(data->lpData);
  editor::Application::instance()->view_event_handler()->OpenFile(
      dom::kInvalidWindowId, file_name);
}

void ApplicationProxy::DidStartChannel(HWND hwnd) {
  DCHECK(!channel_);
  DCHECK(event_);
  channel_.reset(Channel::CreateChannel(hwnd));
  event_->Set();
}

int ApplicationProxy::Run() {
  if (!event_ || !event_->is_existing()) {
    editor::Application::instance()->Run();
    return 0;
  }

  if (!event_->Wait(30 * 1000))
    FatalExit(L"EventObject::Wait");

  std::unique_ptr<Channel> channel(Channel::OpenChannel());
  Channel::ScopedMap payload(channel.get());
  if (!payload)
    FatalExit(L"MapViewOfFile");

  for (auto param : base::CommandLine::ForCurrentProcess()->GetArgs()) {
    base::char16 wsz[MAX_PATH];
    base::char16* pwszFile;
    auto const length = ::GetFullPathNameW(param.c_str(), arraysize(wsz), wsz,
                                           &pwszFile);
    if (!length || length > arraysize(wsz))
     continue;

    COPYDATASTRUCT copy_data;
    copy_data.dwData = 1;
    copy_data.cbData = sizeof(base::char16) * (length + 1);
    copy_data.lpData = wsz;

    ::SendMessage(payload->m_hwnd, WM_COPYDATA, 0,
                  reinterpret_cast<LPARAM>(&copy_data));
  }
  return 0;
}

void ApplicationProxy::WillStartApplication() {
  if (!shell_handler_)
    return;
  shell_handler_->Start();
}

}  // namespace editor
