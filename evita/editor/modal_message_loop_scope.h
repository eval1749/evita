// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_modal_message_loop_scope_h)
#define INCLUDE_evita_editor_modal_message_loop_scope_h

#include "base/basictypes.h"

namespace editor {

// ::MessageBox must be inside ModalMessageLoopScope.
// Note: Windows file dialog box doesn't feed Windows message to MessagePump.
class ModalMessageLoopScope {
  public: ModalMessageLoopScope();
  public: ~ModalMessageLoopScope();
  DISALLOW_COPY_AND_ASSIGN(ModalMessageLoopScope);
};

}  // namespace editor

#endif //!defined(INCLUDE_evita_editor_modal_message_loop_scope_h)
