// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/dom/text/buffer.h"

namespace dom {

Buffer::Buffer(const base::string16& name, text::Mode* mode)
    : text::Buffer(name, mode) {
}

Buffer::~Buffer() {
}

void Buffer::FinishIo(uint const) {
  m_eState = State_Ready;
}

}  // namespace dom
