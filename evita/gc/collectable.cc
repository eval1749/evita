// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/gc/collectable.h"

#include "base/logging.h"
#include "evita/gc/collector.h"
#include "evita/gc/visitor.h"

namespace gc {
namespace internal {

AbstractCollectable::AbstractCollectable()
    : state_(kAlive) {
  Collector::instance().AddToLiveSet(this);
}

AbstractCollectable::~AbstractCollectable() {
  CHECK(is_dead());
}

}  // namespace internal
}  // namespace gc

std::ostream& operator<<(std::ostream& ostream,
                         const gc::internal::AbstractCollectable& object) {
  ostream << (object.is_alive() ? "Alive" : "Dead") << "Collectable@0x" <<
      std::hex << reinterpret_cast<uintptr_t>(&object);
  return ostream;
}
