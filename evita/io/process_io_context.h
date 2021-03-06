// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_PROCESS_IO_CONTEXT_H_
#define EVITA_IO_PROCESS_IO_CONTEXT_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "common/win/scoped_handle.h"
#include "evita//io/block_io_context.h"
#include "evita/dom/public/io_context_id.h"

namespace base {
class Thread;
}

namespace io {

class ProcessIoContext final : public BlockIoContext {
  DECLARE_DEPRECATED_CASTABLE_CLASS(ProcessIoContext, BlockIoContext);

 public:
  ProcessIoContext(domapi::IoContextId context_id,
                   const base::string16& command_line,
                   domapi::OpenProcessPromise promise);
  ~ProcessIoContext();

 private:
  void CloseAndWaitProcess(domapi::IoIntPromise promise);
  uint32_t CloseProcess();
  void ReadFromProcess(void* buffer,
                       size_t num_read,
                       domapi::IoIntPromise promise);
  void StartProcess(domapi::IoContextId context_id,
                    const base::string16& command_line,
                    domapi::OpenProcessPromise promise);
  void WriteToProcess(void* buffer,
                      size_t num_read,
                      domapi::IoIntPromise promise);

  // io::IoContext
  void Close(domapi::IoIntPromise promise) override;
  void Read(void* buffer,
            size_t num_read,
            domapi::IoIntPromise promise) override;
  void Write(void* buffer,
             size_t num_write,
             domapi::IoIntPromise promise) override;

  domapi::IoContextId context_id_;
  std::unique_ptr<base::Thread> gateway_thread_;
  common::win::scoped_handle process_;
  common::win::scoped_handle stdin_write_;
  common::win::scoped_handle stdout_read_;

  DISALLOW_COPY_AND_ASSIGN(ProcessIoContext);
};

}  // namespace io

#endif  // EVITA_IO_PROCESS_IO_CONTEXT_H_
