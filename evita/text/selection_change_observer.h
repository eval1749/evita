// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_SELECTION_CHANGE_OBSERVER_H_
#define EVITA_TEXT_SELECTION_CHANGE_OBSERVER_H_

#include "base/macros.h"

namespace text {

class Selection;

class SelectionChangeObserver {
 public:
  virtual ~SelectionChangeObserver();

  virtual void DidChangeSelection() = 0;

 protected:
  SelectionChangeObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(SelectionChangeObserver);
};

}  // namespace text

#endif  // EVITA_TEXT_SELECTION_CHANGE_OBSERVER_H_
