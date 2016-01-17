// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SELECTION_OBSERVER_H_
#define EVITA_VISUALS_DOM_SELECTION_OBSERVER_H_

#include "base/macros.h"

namespace visuals {

class SelectionModel;

//////////////////////////////////////////////////////////////////////
//
// SelectionObserver
//
class SelectionObserver {
 public:
  ~SelectionObserver();

  virtual void DidChangeCaretBlink() = 0;
  virtual void DidChangeSelection(const SelectionModel& old_model,
                                  const SelectionModel& new_model) = 0;

 protected:
  SelectionObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(SelectionObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SELECTION_OBSERVER_H_
