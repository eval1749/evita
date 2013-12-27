// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/gc/visitable.h"

#include "base/logging.h"
#include "evita/gc/collector.h"
#include "evita/gc/visitor.h"

namespace gc {

void Visitable::Accept(Visitor* visitor) {
  visitor->Visit(this);
}

}  // namespace gc
