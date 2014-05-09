// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_text_composition_event_h)
#define INCLUDE_evita_dom_public_text_composition_event_h

#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_event.h"

namespace domapi {

struct TextCompositionEvent : Event {
  TextCompositionData data;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_text_composition_event_h)
