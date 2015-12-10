// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_BLOCK_IO_CONTEXT_H_
#define EVITA_IO_BLOCK_IO_CONTEXT_H_

#include "evita/io/io_context.h"

namespace io {

class BlockIoContext : public IoContext {
  DECLARE_CASTABLE_CLASS(BlockIoContext, IoContext);

 public:
  virtual ~BlockIoContext();

  virtual void Read(void* buffer,
                    size_t num_read,
                    const domapi::IoIntPromise& promise) = 0;
  virtual void Write(void* buffer,
                     size_t num_write,
                     const domapi::IoIntPromise& promise) = 0;

 protected:
  BlockIoContext();
};

}  // namespace io

#endif  // EVITA_IO_BLOCK_IO_CONTEXT_H_
