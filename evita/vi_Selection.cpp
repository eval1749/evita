#include "precomp.h"
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Selection.h"

#include "evita/text/buffer.h"
#include "evita/text/range.h"

Selection::Selection(text::Range* range)
    : range_(range), start_is_active_(false) {
}

Selection::~Selection() {
}

text::Posn Selection::anchor_offset() const {
  return start_is_active_ ? end() : start();
}

text::Posn Selection::end() const {
  return range_->GetEnd();
}

text::Posn Selection::focus_offset() const {
  return start_is_active_ ? start() : end();
}

text::Posn Selection::start() const {
  return range_->GetStart();
}
