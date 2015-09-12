// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_FILE_IO_CONTEXT_H_
#define EVITA_IO_FILE_IO_CONTEXT_H_

#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/message_loop/message_pump_win.h"
#pragma warning(pop)
#include "common/win/scoped_handle.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/io/io_context.h"

namespace io {

class FileIoContext final : private base::MessagePumpForIO::IOContext,
                            private base::MessagePumpForIO::IOHandler,
                            public IoContext {
 public:
  FileIoContext(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFileDeferred& deferred);
  ~FileIoContext() final;

  bool is_valid() const { return file_handle_.is_valid(); }

 private:
  // base::MessagePumpForIO::IOHandler
  void OnIOCompleted(IOContext* context,
                     DWORD bytes_transfered,
                     DWORD error) override;

  // io::IoContext
  void Close(const domapi::FileIoDeferred& deferred) override;
  void Read(void* buffer,
            size_t num_read,
            const domapi::FileIoDeferred& deferred) override;
  void Write(void* buffer,
             size_t num_write,
             const domapi::FileIoDeferred& deferred) override;

  domapi::FileIoDeferred deferred_;
  common::win::scoped_handle file_handle_;
  bool running_;

  DISALLOW_COPY_AND_ASSIGN(FileIoContext);
};

}  // namespace io

#endif  // EVITA_IO_FILE_IO_CONTEXT_H_
