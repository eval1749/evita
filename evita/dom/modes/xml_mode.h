// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_xml_mode_h)
#define INCLUDE_evita_dom_modes_xml_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class XmlMode : public v8_glue::Scriptable<XmlMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(XmlMode);

  public: XmlMode(Document* document, text::ModeFactory* mode_factory);
  public: ~XmlMode();

  DISALLOW_COPY_AND_ASSIGN(XmlMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_xml_mode_h)
