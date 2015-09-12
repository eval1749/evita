// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_EDITOR_MODAL_MESSAGE_LOOP_SCOPE_H_
#define EVITA_EDITOR_MODAL_MESSAGE_LOOP_SCOPE_H_

#include "base/basictypes.h"

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
