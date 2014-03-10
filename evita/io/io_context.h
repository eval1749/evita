// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_context_h)
#define INCLUDE_evita_io_io_context_h

#include "evita/dom/public/io_callback.h"

namespace io {

class IoContext {
  public: IoContext();
  public: virtual ~IoContext();

  public: virtual void Close(const domapi::FileIoDeferred& deferred) = 0;
  public: virtual void Read(void* buffer, size_t num_read,
                            const domapi::FileIoDeferred& deferred) = 0;
  public: virtual void Write(void* buffer, size_t num_write,
                            const domapi::FileIoDeferred& deferred) = 0;
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_context_h)
