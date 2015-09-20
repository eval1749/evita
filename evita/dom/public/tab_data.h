// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_TAB_DATA_H_
#define EVITA_DOM_PUBLIC_TAB_DATA_H_

#include "base/strings/string16.h"
#include "evita/css/color.h"

namespace domapi {

struct TabData final {
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

  TabData();
  ~TabData();

  bool operator==(const TabData& other) const;
  bool operator!=(const TabData& other) const;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_TAB_DATA_H_
