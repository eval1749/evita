#include "precomp.h"
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Selection.h"

#include "base/strings/string16.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/text/range.h"
#include "evita/vi_TextEditWindow.h"

Selection::Selection(dom::Buffer* buffer, text::Range* range)
    : m_fStartIsActive(false),
      m_pBuffer(buffer),
      m_pRange(range) {
}

Selection::~Selection() {
}

Posn Selection::GetEnd() const {
  return m_pRange->GetEnd();
}

Posn Selection::GetStart() const {
  return m_pRange->GetStart();
}

void Selection::SetStartIsActive(bool new_start_is_active) {
  m_fStartIsActive = new_start_is_active;
}
