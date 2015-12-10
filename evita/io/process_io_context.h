// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_PROCESS_IO_CONTEXT_H_
#define EVITA_IO_PROCESS_IO_CONTEXT_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/io_context_id.h"
#include "evita//io/block_io_context.h"

namespace base {
class Thread;
}

namespace io {

class ProcessIoContext final : public BlockIoContext {
  DECLARE_CASTABLE_CLASS(ProcessIoContext, BlockIoContext);

 public:
  ProcessIoContext(domapi::IoContextId context_id,
                   const base::string16& command_line,
                   const domapi::OpenProcessDeferred& deferred);
  ~ProcessIoContext();

 private:
  void CloseAndWaitProcess(const domapi::FileIoDeferred& deferred);
  uint32_t CloseProcess();
  void ReadFromProcess(void* buffer,
                       size_t num_read,
                       const domapi::FileIoDeferred& deferred);
  void StartProcess(domapi::IoContextId context_id,
                    const base::string16& command_line,
                    const domapi::OpenProcessDeferred& deferred);
  void WriteToProcess(void* buffer,
                      size_t num_read,
                      const domapi::FileIoDeferred& deferred);

  // io::IoContext
  void Close(const domapi::FileIoDeferred& deferred) override;
  void Read(void* buffer,
            size_t num_read,
            const domapi::FileIoDeferred& deferred) override;
  void Write(void* buffer,
             size_t num_write,
             const domapi::FileIoDeferred& deferred) override;

  domapi::IoContextId context_id_;
  std::unique_ptr<base::Thread> gateway_thread_;
  common::win::scoped_handle process_;
  common::win::scoped_handle stdin_write_;
  common::win::scoped_handle stdout_read_;

  DISALLOW_COPY_AND_ASSIGN(ProcessIoContext);
};

}  // namespace io

#endif  // EVITA_IO_PROCESS_IO_CONTEXT_H_
