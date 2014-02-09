// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/mode.h"

#include "evita/dom/modes/mode_factory.h"

namespace text {

Mode::Mode(ModeFactory* pClass, Buffer* pBuffer) :
    m_pBuffer(pBuffer),
    m_pClass(pClass) {
}

Mode::~Mode() {
}

uint32_t Mode::GetCharSyntax(base::char16 wch) const {
  return m_pClass->GetCharSyntax(wch);
}

const char16* Mode::GetName() const {
  return m_pClass->GetName();
}

}  // namespace text
