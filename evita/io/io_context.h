// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_CONTEXT_H_
#define EVITA_IO_IO_CONTEXT_H_

#include "base/macros.h"
#include "evita/base/castable.h"
#include "evita/dom/public/io_callback.h"

namespace io {

//////////////////////////////////////////////////////////////////////
//
// IoContext
//
class IoContext : public base::DeprecatedCastable<IoContext> {
  DECLARE_DEPRECATED_CASTABLE_CLASS(IoContext, DeprecatedCastable);

 public:
  virtual ~IoContext();

  virtual void Close(const domapi::IoIntPromise& promise) = 0;

 protected:
  IoContext();

 private:
  DISALLOW_COPY_AND_ASSIGN(IoContext);
};

}  // namespace io

#endif  // EVITA_IO_IO_CONTEXT_H_
