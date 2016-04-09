// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/modal_message_loop_scope.h"

#include "base/logging.h"

namespace editor {

ModalMessageLoopScope::ModalMessageLoopScope()
    : allow_nested_(base::MessageLoop::current()) {
  DCHECK(!base::MessageLoop::current()->os_modal_loop());
  base::MessageLoop::current()->set_os_modal_loop(true);
}

ModalMessageLoopScope::~ModalMessageLoopScope() {
  base::MessageLoop::current()->set_os_modal_loop(false);
}

}  // namespace editor
