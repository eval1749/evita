// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_MODAL_MESSAGE_LOOP_SCOPE_H_
#define EVITA_EDITOR_MODAL_MESSAGE_LOOP_SCOPE_H_

#include "base/macros.h"

namespace editor {

// ::MessageBox must be inside ModalMessageLoopScope.
// Note: Windows file dialog box doesn't feed Windows message to MessagePump.
class ModalMessageLoopScope final {
 public:
  ModalMessageLoopScope();
  ~ModalMessageLoopScope();

 private:
  DISALLOW_COPY_AND_ASSIGN(ModalMessageLoopScope);
};

}  // namespace editor

#endif  // EVITA_EDITOR_MODAL_MESSAGE_LOOP_SCOPE_H_
