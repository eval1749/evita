// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/view_event_handler.h"

namespace domapi {

ViewEventHandler::ViewEventHandler() {
}

ViewEventHandler::~ViewEventHandler() {
}

void ViewEventHandler::DidChangeWindowVisibility(WindowId, Visibility) {
  // TODO(eval1749) This is stab. Once we finish view side implementation,
  // we'll remove this.
}

}  // namespace domapi
