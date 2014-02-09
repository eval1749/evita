// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode.h"

#include "evita/text/modes/mode_factory.h"

namespace text {

Mode::Mode(ModeFactory* pClass, Buffer* pBuffer)
    : m_pBuffer(pBuffer),
      m_pClass(pClass) {
}

Mode::~Mode() {
}

const char16* Mode::GetName() const {
  return m_pClass->GetName();
}

}  // namespace text
