// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_id_h)
#define INCLUDE_evita_dom_window_id_h

#include "evita/dom/public/event_target_id.h"

namespace dom {

typedef domapi::EventTargetId WindowId;
const WindowId kInvalidWindowId = domapi::kInvalidEventTargetId;

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_window_id_h)
