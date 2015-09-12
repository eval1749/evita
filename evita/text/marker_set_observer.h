// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_MARKER_SET_OBSERVER_H_
#define EVITA_TEXT_MARKER_SET_OBSERVER_H_

#include "evita/precomp.h"

namespace text {

class MarkerSetObserver {
 public:
  MarkerSetObserver();
  virtual ~MarkerSetObserver();

  virtual void DidChangeMarker(Posn start, Posn end) = 0;
};

}  // namespace text

#endif  // EVITA_TEXT_MARKER_SET_OBSERVER_H_
