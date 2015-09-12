// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_MANAGER_H_
#define EVITA_IO_IO_MANAGER_H_

#include <windows.h>
#include <memory>

#include "base/basictypes.h"

namespace base {
class MessageLoopForIO;
class Thread;
}

namespace domapi {
class IoDelegate;
}

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
class IoManager final {
 public:
  IoManager();
  ~IoManager();

  domapi::IoDelegate* io_delegate() const;
  base::MessageLoopForIO* message_loop() const;

  void RegisterIoHandler(HANDLE handle, void* io_handler);
  void Start();

 private:
  std::unique_ptr<domapi::IoDelegate> io_delegate_;
  std::unique_ptr<base::Thread> io_thread_;

  DISALLOW_COPY_AND_ASSIGN(IoManager);
};

#endif  // EVITA_IO_IO_MANAGER_H_
