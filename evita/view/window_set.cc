// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/window_set.h"

namespace view {

WindowSet::WindowSet(Set&& set)
    : set_(set) {
}

WindowSet::~WindowSet() {
}

}  // namespace view
