// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/public/view_lifecycle_client.h"

#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycleClient
//
ViewLifecycleClient::ViewLifecycleClient(ViewLifecycle* lifecycle)
    : lifecycle_(lifecycle) {}

ViewLifecycleClient::~ViewLifecycleClient() {}

const Document& ViewLifecycleClient::document() const {
  return lifecycle_->document();
}

const css::Media& ViewLifecycleClient::media() const {
  return lifecycle_->media();
}

}  // namespace visuals
