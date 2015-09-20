// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_TEXT_COMPOSITION_DATA_H_
#define EVITA_DOM_PUBLIC_TEXT_COMPOSITION_DATA_H_

#include <vector>

#include "base/strings/string16.h"

namespace domapi {

struct TextCompositionSpan final {
  int start;
  int end;
  int data;
};

struct TextCompositionData final {
  int caret;
  base::string16 text;
  std::vector<TextCompositionSpan> spans;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_TEXT_COMPOSITION_DATA_H_
