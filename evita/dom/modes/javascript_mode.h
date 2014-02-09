// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_javascript_mode_h)
#define INCLUDE_evita_dom_modes_javascript_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class JavaScriptMode : public v8_glue::Scriptable<JavaScriptMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(JavaScriptMode);

  public: JavaScriptMode();
  public: ~JavaScriptMode();

  DISALLOW_COPY_AND_ASSIGN(JavaScriptMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_javascript_mode_h)
