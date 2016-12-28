// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUE_FORWARD_H_
#define JOANA_ANALYZER_VALUE_FORWARD_H_

namespace joana {
namespace analyzer {

#define FOR_EACH_ANALYZE_PRIMITIVE_TYPES(name) \
  V(boolean)                                   \
  V(function)                                  \
  V(null)                                      \
  V(number)                                    \
  V(object)                                    \
  V(string)  /* NOLINT */                      \
  V(symbol)                                    \
  V(undefined)                                 \
  V(unknown)                                   \
  V(void)

#define FOR_EACH_ANALYZE_VALUE(name) \
  V(Function)                        \
  V(Property)                        \
  V(Variable)

#define V(name) class name;
FOR_EACH_ANALYZE_VALUE(V)
#undef V

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUE_FORWARD_H_
