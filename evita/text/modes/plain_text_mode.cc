// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/plain_text_mode.h"

namespace text {

PlainTextMode::PlainTextMode() {
}

PlainTextMode::~PlainTextMode() {
}

bool PlainTextMode::DoColor(Count) {
  return false;
}

const char16* PlainTextMode::GetName() const {
  return L"Plain";
}

}  // namespace text
