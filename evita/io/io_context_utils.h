// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_CONTEXT_UTILS_H_
#define EVITA_IO_IO_CONTEXT_UTILS_H_

#include <stdint.h>

#include "base/callback_forward.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"

namespace io {

void Reject(const base::Callback<void(domapi::IoError)>& reject,
            uint32_t last_error);

void Resolve(const base::Callback<void(int)>& resolve,
             uint32_t num_transferred);

void RunCallback(const base::Closure& callback);

}  // namespace io

#endif  // EVITA_IO_IO_CONTEXT_UTILS_H_
