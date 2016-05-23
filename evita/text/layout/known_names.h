// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_KNOWN_NAMES_H_
#define EVITA_TEXT_LAYOUT_KNOWN_NAMES_H_

#include "evita/base/strings/atomic_string.h"

namespace layout {

#define FOR_EACH_KNOWN_NAME(V)   \
  V("::default", cc_default)     \
  V("::ruler", cc_ruler)         \
  V("::selection", cc_selection) \
  V(":active", c_active)         \
  V(":inactive", c_inactive)     \
  V("control", control)          \
  V("marker", marker)            \
  V("missing", missing)          \
  V("normal", normal)

class KnownNames final {
 public:
  KnownNames();
  ~KnownNames() = default;

#define V(string, name) \
  base::AtomicString name() const { return name##_; }
  FOR_EACH_KNOWN_NAME(V)
#undef V

  static KnownNames* GetInstance();

 private:
#define V(string, name) const base::AtomicString name##_;
  FOR_EACH_KNOWN_NAME(V)
#undef V
  int dummy_;
};

#define KNOWN_NAME_OF(name) (KnownNames::GetInstance()->name())

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_KNOWN_NAMES_H_
