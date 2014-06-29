// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/selection.h"

#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace text {

Selection::Selection(Range* range)
    : range_(range), start_is_active_(false) {
}

Selection::~Selection() {
}

Posn Selection::anchor_offset() const {
  return start_is_active_ ? end() : start();
}

Posn Selection::end() const {
  return range_->end();
}

Posn Selection::focus_offset() const {
  return start_is_active_ ? start() : end();
}

Posn Selection::start() const {
  return range_->start();
}

}  // namespace text
