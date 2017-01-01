// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "joana/testing/simple_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/literals.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"
#include "joana/base/source_code.h"

namespace joana {
namespace parser {

namespace {

//
// Context
//
struct Context {
  int depth = 0;

  Context() = default;
};

struct Printable {
  Context* context;
  const ast::Node* node;
};

std::ostream& operator<<(std::ostream& ostream, const Printable& printable) {
  const auto& node = printable.node;
  return ostream << node;
}

}  // namespace

Formatted AsFormatted(const ast::Node& node) {
  return Formatted{&node};
}

std::ostream& operator<<(std::ostream& ostream, const Formatted& formatted) {
  Context context;
  return ostream << Printable{&context, formatted.node};
}

}  // namespace parser
}  // namespace joana
