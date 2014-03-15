// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_mason_mode_h)
#define INCLUDE_evita_dom_modes_mason_mode_h

#include "evita/dom/text/modes/mode.h"

namespace dom {

class MasonMode : public v8_glue::Scriptable<MasonMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(MasonMode);

  public: MasonMode();
  public: ~MasonMode();

  DISALLOW_COPY_AND_ASSIGN(MasonMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_mason_mode_h)
