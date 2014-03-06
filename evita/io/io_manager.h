// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_manager_h)
#define INCLUDE_evita_io_io_manager_h

namespace base {
class Time;
}

#include "common/win/native_window.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/view_delegate.h"

namespace base {
class MessageLoop;
class Thread;
}

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
class IoManager : public common::win::NativeWindow {
  private: std::unique_ptr<domapi::IoDelegate> io_delegate_;
  private: std::unique_ptr<base::Thread> io_thread_;

  public: IoManager();
  public: virtual ~IoManager();

  public: domapi::IoDelegate* io_delegate() const;
  public: base::MessageLoop* message_loop() const;

  public: void RegisterIoHandler(HANDLE handle, void* io_handler);
  public: void Start();

  // common::win::NativeWindow
  private: virtual LRESULT WindowProc(UINT, WPARAM, LPARAM) override;

  DISALLOW_COPY_AND_ASSIGN(IoManager);
};

//////////////////////////////////////////////////////////////////////
//
// SharedArea
//
struct SharedArea {
  HWND m_hwnd;
  char16 m_wsz[1];
}; // SharedArea

const char16 k_wszFileMapping[] =
    L"Local\\03002DEC-D63E-4551-9AE8-B88E8C586376";

const uint k_cbFileMapping = 1024 * 64;

extern HANDLE g_hEvent;
extern bool g_fMultiple;

#endif //!defined(INCLUDE_evita_io_io_manager_h)
