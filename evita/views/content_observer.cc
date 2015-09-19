// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/content_observer.h"

#include "base/logging.h"

namespace views {

ContentObserver::ContentObserver() {}

ContentObserver::~ContentObserver() {}

void ContentObserver::DidUpdateContent(ContentWindow* window) {
  DCHECK(window);
}

}  // namespace views
