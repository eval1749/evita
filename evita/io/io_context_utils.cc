// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_context_utils.h"

#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"

namespace io {

void Reject(base::OnceCallback<void(domapi::IoError)> reject,
            uint32_t last_error) {
  RunCallback(base::BindOnce(std::move(reject),
                             domapi::IoError(static_cast<int>(last_error))));
}

void Resolve(base::OnceCallback<void(int)> resolve, uint32_t num_transferred) {
  RunCallback(
      base::BindOnce(std::move(resolve), static_cast<int>(num_transferred)));
}

void RunCallback(base::OnceClosure callback) {
  editor::Application::instance()->view_event_handler()->RunCallback(
      std::move(callback));
}

}  // namespace io
