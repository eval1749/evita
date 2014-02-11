// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode.h"

#include "base/logging.h"
#include "evita/text/modes/lexer.h"

namespace text {

Mode::Mode() : buffer_(nullptr) {
}

Mode::~Mode() {
}

void Mode::set_buffer(Buffer* buffer) {
  DCHECK(!buffer_);
  DCHECK(buffer);
  buffer_ = buffer;
}

ModeWithLexer::ModeWithLexer() {
}

ModeWithLexer::~ModeWithLexer() {
}

bool ModeWithLexer::DoColor(Count hint) {
  if (!lexer_)
    lexer_.reset(CreateLexer(buffer()));
  return lexer_->Run(hint);
}

}  // namespace text
