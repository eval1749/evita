// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_FILE_IO_CONTEXT_H_
#define EVITA_IO_FILE_IO_CONTEXT_H_

#include "base/callback.h"
#include "base/message_loop/message_pump_win.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/io/block_io_context.h"

namespace io {

class FileIoContext final : private base::MessagePumpForIO::IOContext,
                            private base::MessagePumpForIO::IOHandler,
                            public BlockIoContext {
  DECLARE_CASTABLE_CLASS(FileIoContext, BlockIoContext);

 public:
  FileIoContext(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFilePromise& deferred);
  ~FileIoContext() final;

  bool is_valid() const { return file_handle_.is_valid(); }

 private:
  bool IsRunning() const { return operation_ != nullptr; }

  // base::MessagePumpForIO::IOHandler
  void OnIOCompleted(IOContext* context,
                     DWORD bytes_transfered,
                     DWORD error) override;

  // io::IoContext
  void Close(const domapi::IoIntPromise& deferred) override;
  void Read(void* buffer,
            size_t num_read,
            const domapi::IoIntPromise& deferred) override;
  void Write(void* buffer,
             size_t num_write,
             const domapi::IoIntPromise& deferred) override;

  domapi::IoIntPromise deferred_;
  common::win::scoped_handle file_handle_;
  const char* operation_;

  DISALLOW_COPY_AND_ASSIGN(FileIoContext);
};

}  // namespace io

#endif  // EVITA_IO_FILE_IO_CONTEXT_H_
