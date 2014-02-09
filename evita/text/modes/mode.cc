// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode.h"

#include "base/logging.h"

namespace text {

Mode::Mode() : buffer_(nullptr) {
}

Mode::~Mode() {
}

void Mode::set_buffer(Buffer* buffer) {
  DCHECK(!buffer_);
  DCHECK(buffer);
  buffer_ = buffer;
}

}  // namespace text
