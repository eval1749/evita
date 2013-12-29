// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/dialog_box.h"

#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable: 4100)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)

namespace editor {

ModalMessageLoopScope::ModalMessageLoopScope() {
  DCHECK(!base::MessageLoop::current()->os_modal_loop());
  base::MessageLoop::current()->set_os_modal_loop(true);
}

ModalMessageLoopScope:: ~ModalMessageLoopScope() {
  base::MessageLoop::current()->set_os_modal_loop(false);
}

}  // namespace editor
