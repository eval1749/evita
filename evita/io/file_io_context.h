// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_FILE_IO_CONTEXT_H_
#define EVITA_IO_FILE_IO_CONTEXT_H_

#include <utility>

#include "base/callback.h"
#include "base/macros.h"
#include "base/message_loop/message_pump_win.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/promise.h"
#include "evita/io/block_io_context.h"

namespace io {

class FileIoContext final : private base::MessagePumpForIO::IOContext,
                            private base::MessagePumpForIO::IOHandler,
                            public BlockIoContext {
  DECLARE_DEPRECATED_CASTABLE_CLASS(FileIoContext, BlockIoContext);

 public:
  explicit FileIoContext(HANDLE handle);
  ~FileIoContext() final;

  static std::pair<HANDLE, int> Open(const base::string16& file_name,
                                     const base::string16& mode);

 private:
  bool IsRunning() const { return operation_ != nullptr; }

  // base::MessagePumpForIO::IOHandler
  void OnIOCompleted(IOContext* context,
                     DWORD bytes_transfered,
                     DWORD error) override;

  // io::IoContext
  void Close(domapi::IoIntPromise promise) override;
  void Read(void* buffer,
            size_t num_read,
            domapi::IoIntPromise promise) override;
  void Write(void* buffer,
             size_t num_write,
             domapi::IoIntPromise promise) override;

  domapi::IoIntPromise promise_;
  common::win::scoped_handle file_handle_;
  const char* operation_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(FileIoContext);
};

}  // namespace io

#endif  // EVITA_IO_FILE_IO_CONTEXT_H_
