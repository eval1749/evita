// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_mode_factory_h)
#define INCLUDE_evita_text_modes_mode_factory_h

#include "base/basictypes.h"

namespace text {

class Mode;

class ModeFactory {
  protected: ModeFactory();
  public: virtual ~ModeFactory();

  // [C]
  public: virtual Mode* Create(Buffer*) = 0;

  // [E]
  protected: virtual const char16* getExtensions() const { return L""; }

  // [G]
  public: virtual const char16* GetName() const = 0;

  // [I]
  public: virtual bool IsSupported(const char16*) const;
};

} // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mode_factory_h)
