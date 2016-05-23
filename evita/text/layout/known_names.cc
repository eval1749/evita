// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/layout/known_names.h"

#include "base/memory/singleton.h"

namespace layout {

#define V(string, name) name##_(L##string),
KnownNames::KnownNames() : FOR_EACH_KNOWN_NAME(V) dummy_(0) {}
#undef V

// static
KnownNames* KnownNames::GetInstance() {
  return base::Singleton<KnownNames>::get();
}

}  // namespace layout
