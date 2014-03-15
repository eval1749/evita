// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_perl_mode_h)
#define INCLUDE_evita_dom_modes_perl_mode_h

#include "evita/dom/text/modes/mode.h"

namespace dom {

class PerlMode : public v8_glue::Scriptable<PerlMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(PerlMode);

  public: PerlMode();
  public: ~PerlMode();

  DISALLOW_COPY_AND_ASSIGN(PerlMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_perl_mode_h)
