// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/dialog_box.h"

#include "base/logging.h"
// L4 C4100: 'identifier' : unreferenced formal parameter
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
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
