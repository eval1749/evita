// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_manager.h"

#pragma warning(push)
#pragma warning(disable: 4365)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "base/time/time.h"
#include "common/win/point.h"
#include "common/win/size.h"
#include "evita/dom/buffer.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_delegate_impl.h"
#include "evita/views/window_id.h"

HANDLE g_hEvent;
bool g_fMultiple;

// TODO(yosi) Once we move IoManager to namespace |io|, we should remove this
// |using| statement.
using io::IoDelegateImpl;

namespace {

enum Message {
  Message_Start = WM_USER,
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
IoManager::IoManager()
    : io_delegate_(new IoDelegateImpl()),
      io_thread_(new base::Thread("io_manager_thread")) {
}

IoManager::~IoManager() {
}

domapi::IoDelegate* IoManager::io_delegate() const {
  return io_delegate_.get();
}

base::MessageLoop* IoManager::message_loop() const {
  return io_thread_->message_loop();
}

void IoManager::Start() {
  CreateWindowEx(0, 0, L"IoManager", HWND_MESSAGE, common::win::Point(),
                 common::win::Size());
  DCHECK(*this);
  io_thread_->StartWithOptions(
      base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
}

// domapi::IoDelegate
void IoManager::QueryFileStatus(const base::string16& filename,
                                const QueryFileStatusCallback& callback) {
  message_loop()->PostTask(FROM_HERE, base::Bind(
      &IoDelegate::QueryFileStatus, base::Unretained(io_delegate_.get()),
      filename, callback));
}

// common::win::NativeWindow
LRESULT IoManager::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CREATE: {
      if (g_fMultiple)
        break;
      auto const hMapping = ::CreateFileMapping(
          INVALID_HANDLE_VALUE, // hFile
          nullptr, // lpAttributes
          PAGE_READWRITE, // flProtect
          0, // dwMaximumSizeHigh
          k_cbFileMapping, // dwMaximumSizeLow
          k_wszFileMapping); // lpName

      DCHECK(hMapping);

      auto const p = reinterpret_cast<SharedArea*>(::MapViewOfFile(
          hMapping,
          FILE_MAP_READ | FILE_MAP_WRITE,
          0, // dwFileOffsetHigh
          0, // dwFileOffsetLow
          k_cbFileMapping));

      if (p)
        p->m_hwnd = *this;
      ::SetEvent(g_hEvent);
      break;
    }

    case WM_COPYDATA: {
      auto const p = reinterpret_cast<COPYDATASTRUCT*>(lParam);
      auto const filename = reinterpret_cast<base::char16*>(p->lpData);
      Application::instance()->view_event_handler()->OpenFile(
          views::kInvalidWindowId, filename);
      // TODO(yosi) Should we call |SetForegroundWindow|?
      return true;
    }
  }
  return NativeWindow::WindowProc(uMsg, wParam, lParam);
}
