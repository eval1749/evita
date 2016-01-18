// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_CLIENT_H_
#define EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_CLIENT_H_

#include "base/macros.h"

namespace visuals {

class Document;
class ViewLifecycle;

namespace css {
class Media;
}

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycleClient
//
class ViewLifecycleClient {
 public:
  ~ViewLifecycleClient();

  const Document& document() const;
  const ViewLifecycle& lifecycle() const { return *lifecycle_; }
  ViewLifecycle* lifecycle() { return lifecycle_; }
  const css::Media& media() const;

 protected:
  explicit ViewLifecycleClient(ViewLifecycle* lifecycle);

 private:
  ViewLifecycle* const lifecycle_;

  DISALLOW_COPY_AND_ASSIGN(ViewLifecycleClient);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_CLIENT_H_
