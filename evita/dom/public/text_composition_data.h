// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_text_composition_data_h)
#define INCLUDE_evita_dom_public_text_composition_data_h

#include <vector>

#include "base/strings/string16.h"

namespace domapi {

struct TextCompositionData {
  std::vector<uint8_t> attributes;
  int caret;
  base::string16 text;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_text_composition_data_h)
