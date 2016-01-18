// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_OBSERVER_H_
#define EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_OBSERVER_H_

#include "base/macros.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycleObserver
//
class ViewLifecycleObserver {
 public:
  virtual ~ViewLifecycleObserver();

  virtual void DidChangeLifecycleState(ViewLifecycle::State new_state,
                                       ViewLifecycle::State old_state) = 0;

 protected:
  ViewLifecycleObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewLifecycleObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_OBSERVER_H_
