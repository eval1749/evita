// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_file_io_context_h)
#define INCLUDE_evita_io_file_io_context_h

#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_pump_win.h"
#pragma warning(pop)
#include "common/win/scoped_handle.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/io/io_context.h"

namespace io {

class FileIoContext : private base::MessagePumpForIO::IOContext,
                      private base::MessagePumpForIO::IOHandler,
                      public IoContext {
  private: domapi::FileIoDeferred deferred_;
  private: common::win::scoped_handle file_handle_;
  private: bool running_;

  public: FileIoContext(const base::string16& file_name,
                        const base::string16& mode,
                        const domapi::OpenFileCallback& callback);
  public: virtual ~FileIoContext();

  public: bool is_valid() const { return file_handle_.is_valid(); }

  // base::MessagePumpForIO::IOHandler
  private: virtual void OnIOCompleted(
      IOContext* context, DWORD bytes_transfered, DWORD error) override;

  // io::IoContext
  private: virtual void Close(
      const domapi::FileIoDeferred& deferred) override;
  private: virtual void Read(void* buffer, size_t num_read,
                             const domapi::FileIoDeferred& deferred) override;
  private: virtual void Write(void* buffer, size_t num_write,
                             const domapi::FileIoDeferred& deferred) override;

  DISALLOW_COPY_AND_ASSIGN(FileIoContext);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_file_io_context_h)
