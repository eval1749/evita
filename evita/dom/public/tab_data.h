// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_tab_data_h)
#define INCLUDE_evita_dom_public_tab_data_h

#include "base/strings/string16.h"
#include "evita/css/color.h"

namespace domapi {

struct TabData {
  enum class State {
    Normal,
    Modified,
    Loading,
    Saving,
    Steal,
  };

  int icon;
  State state;
  base::string16 title;
  base::string16 tooltip;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_tab_data_h)
