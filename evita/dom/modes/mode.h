// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_mode_h)
#define INCLUDE_evita_dom_modes_mode_h

#include <memory>

#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Mode;
}

namespace dom {

class Document;

class Mode : public v8_glue::Scriptable<Mode> {
  DECLARE_SCRIPTABLE_OBJECT(Mode);
  private: std::unique_ptr<text::Mode> mode_;

  protected: Mode(text::Mode* text_mode);
  public: ~Mode();

  public: text::Mode* text_mode() const { return mode_.get(); }
  public: base::string16 name() const;

  public: void DoColor(int hint);

  DISALLOW_COPY_AND_ASSIGN(Mode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_mode_h)
