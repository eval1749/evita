// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_WINDOW_ID_H_
#define EVITA_DOM_WINDOWS_WINDOW_ID_H_

#include "evita/dom/public/event_target_id.h"

namespace dom {

typedef domapi::EventTargetId WindowId;
const WindowId kInvalidWindowId = domapi::kInvalidEventTargetId;

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_WINDOW_ID_H_
