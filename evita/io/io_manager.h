// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_manager_h)
#define INCLUDE_evita_io_io_manager_h

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
class IoManager {
  private: std::unique_ptr<domapi::IoDelegate> io_delegate_;
  private: std::unique_ptr<base::Thread> io_thread_;

  public: IoManager();
  public: virtual ~IoManager();

  public: domapi::IoDelegate* io_delegate() const;
  public: base::MessageLoopForIO* message_loop() const;

  public: void RegisterIoHandler(HANDLE handle, void* io_handler);
  public: void Start();

  DISALLOW_COPY_AND_ASSIGN(IoManager);
};

#endif //!defined(INCLUDE_evita_io_io_manager_h)
