// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_manager.h"

#pragma warning(push)
#pragma warning(disable : 4365)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "base/time/time.h"
#include "common/win/point.h"
#include "common/win/size.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_delegate_impl.h"
#include "evita/io/io_thread_proxy.h"
#include "evita/views/window_id.h"

HANDLE g_hEvent;
bool g_fMultiple;

namespace io {

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
      io_thread_(new base::Thread("io_manager_thread")),
      proxy_(new IoThreadProxy(io_delegate_.get(), io_thread_.get())) {}

IoManager::~IoManager() {}

base::MessageLoopForIO* IoManager::message_loop() const {
  return static_cast<base::MessageLoopForIO*>(io_thread_->message_loop());
}

domapi::IoDelegate* IoManager::proxy() const {
  return proxy_.get();
}

void IoManager::RegisterIoHandler(HANDLE handle, void* io_handler) {
  DCHECK(handle);
  DCHECK(io_handler);
  message_loop()->RegisterIOHandler(
      handle, reinterpret_cast<base::MessageLoopForIO::IOHandler*>(io_handler));
}

void IoManager::Start() {
  io_thread_->StartWithOptions(
      base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
}

}  // namespace io
