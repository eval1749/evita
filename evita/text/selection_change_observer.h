// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_selection_change_observer_h)
#define INCLUDE_evita_text_selection_change_observer_h

#include "base/macros.h"

namespace text {

class Selection;

class SelectionChangeObserver {
  public: SelectionChangeObserver();
  public: virtual ~SelectionChangeObserver();

  public: virtual void DidChangeSelection() = 0;

  DISALLOW_COPY_AND_ASSIGN(SelectionChangeObserver);
};

}   // text

#endif //!defined(INCLUDE_evita_text_selection_change_observer_h)
