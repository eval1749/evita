// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_CONTEXT_H_
#define EVITA_IO_IO_CONTEXT_H_

#include "common/castable.h"
#include "evita/dom/public/io_callback.h"

namespace io {

class IoContext : public common::Castable {
  DECLARE_CASTABLE_CLASS(IoContext, Castable);

 public:
  virtual ~IoContext();

  virtual void Close(const domapi::IoIntPromise& deferred) = 0;

 protected:
  IoContext();
};

}  // namespace io

#endif  // EVITA_IO_IO_CONTEXT_H_
