// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_cxx_mode_h)
#define INCLUDE_evita_dom_modes_cxx_mode_h

#include "evita/dom/text/modes/mode.h"

namespace dom {

class CxxMode : public v8_glue::Scriptable<CxxMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(CxxMode);

  public: CxxMode();
  public: ~CxxMode();

  DISALLOW_COPY_AND_ASSIGN(CxxMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_cxx_mode_h)
