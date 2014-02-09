// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_config_mode_h)
#define INCLUDE_evita_dom_modes_config_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class ConfigMode : public v8_glue::Scriptable<ConfigMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(ConfigMode);

  public: ConfigMode();
  public: ~ConfigMode();

  DISALLOW_COPY_AND_ASSIGN(ConfigMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_config_mode_h)
