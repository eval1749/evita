// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_TEXT_COMPOSITION_EVENT_H_
#define EVITA_DOM_PUBLIC_TEXT_COMPOSITION_EVENT_H_

#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_events.h"

namespace domapi {

struct TextCompositionEvent final : Event {
  TextCompositionData data;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_TEXT_COMPOSITION_EVENT_H_
