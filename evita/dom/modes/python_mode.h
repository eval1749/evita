// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_python_mode_h)
#define INCLUDE_evita_dom_modes_python_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class PythonMode : public v8_glue::Scriptable<PythonMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(PythonMode);

  public: PythonMode(Document* document, text::ModeFactory* mode_factory);
  public: ~PythonMode();

  DISALLOW_COPY_AND_ASSIGN(PythonMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_python_mode_h)
