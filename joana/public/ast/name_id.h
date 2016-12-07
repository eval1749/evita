// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NAME_ID_H_
#define JOANA_PUBLIC_AST_NAME_ID_H_

#include "joana/public/ast/lexical_grammar.h"

namespace joana {
namespace ast {

enum class NameId {
  Zero,

  StartKeyword,
#define V(name, camel, upper) Keyword##camel,
  FOR_EACH_JAVASCRIPT_KEYWORD(V)
#undef V
      EndKwyrod,

  StartKnown,
#define V(name, camel, upper) Known##camel,
  FOR_EACH_JAVASCRIPT_KNOWN_WORD(V)
#undef V
      EndKnown,
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NAME_ID_H_
