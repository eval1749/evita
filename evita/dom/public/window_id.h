// Copyright (c) 1996-2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_WINDOW_ID_H_
#define EVITA_DOM_PUBLIC_WINDOW_ID_H_

#include "evita/dom/public/event_target_id.h"

namespace domapi {

using WindowId = EventTargetId;
const WindowId kInvalidWindowId = kInvalidEventTargetId;

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_WINDOW_ID_H_
