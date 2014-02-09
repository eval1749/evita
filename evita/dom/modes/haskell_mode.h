// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_haskell_mode_h)
#define INCLUDE_evita_dom_modes_haskell_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class HaskellMode : public v8_glue::Scriptable<HaskellMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(HaskellMode);

  public: HaskellMode();
  public: ~HaskellMode();

  DISALLOW_COPY_AND_ASSIGN(HaskellMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_haskell_mode_h)
