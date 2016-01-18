// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_VIEW_OBSERVER_H_
#define EVITA_VISUALS_VIEW_VIEW_OBSERVER_H_

#include "base/macros.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ViewObserver
//
class ViewObserver {
 public:
  virtual ~ViewObserver();

  virtual void DidChangeView() = 0;

 protected:
  ViewObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_VIEW_OBSERVER_H_
