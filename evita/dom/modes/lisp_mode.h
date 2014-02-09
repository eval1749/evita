// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_lisp_mode_h)
#define INCLUDE_evita_dom_modes_lisp_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class LispMode : public v8_glue::Scriptable<LispMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(LispMode);

  public: LispMode();
  public: ~LispMode();

  DISALLOW_COPY_AND_ASSIGN(LispMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_lisp_mode_h)
