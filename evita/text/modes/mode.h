// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_mode_h)
#define INCLUDE_evita_text_modes_mode_h

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace text {

class Buffer;
class Mode;
class ModeFactory;

class Mode {
  protected: Buffer* m_pBuffer;
  protected: ModeFactory* m_pClass;

  protected: Mode(ModeFactory*, Buffer*);
  public: virtual ~Mode();

  // [D]
  public: virtual bool DoColor(Count) = 0;

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }

  public: ModeFactory* GetClass() const { return m_pClass; }
  public: virtual const char16* GetName() const;

  DISALLOW_COPY_AND_ASSIGN(Mode);
};

} // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mode_h)
