// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_text_composition_data_h)
#define INCLUDE_evita_dom_public_text_composition_data_h

#include <vector>

#include "base/strings/string16.h"

namespace domapi {

struct TextCompositionSpan {
  int start;
  int end;
  int data;
};

struct TextCompositionData {
  int caret;
  base::string16 text;
  std::vector<TextCompositionSpan> spans;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_text_composition_data_h)
