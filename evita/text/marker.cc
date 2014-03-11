// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker.h"

namespace text {

Marker::Marker(int type, Posn start, Posn end)
    : end_(end), start_(start), type_(type) {
  DCHECK_NE(Marker::None, type);
  DCHECK_LT(start, end);
}

Marker::Marker(Posn offset)
    : end_(offset), start_(offset), type_(Marker::None) {
}

Marker::Marker() : Marker(0) {
}

Marker::~Marker() {
}

}  // namespace text
