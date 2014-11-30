// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_context_utils.h"

#include "base/bind.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"

namespace io {

void Reject(const base::Callback<void(domapi::IoError)>& reject,
            DWORD last_error) {
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(reject, domapi::IoError(static_cast<int>(last_error))));
}

void Resolve(const base::Callback<void(int)>& resolve,
             DWORD num_transferred) {
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve , static_cast<int>(num_transferred)));
}

}  // namespace io
