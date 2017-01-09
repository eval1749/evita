// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_printer.h"

#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/node.h"

namespace joana {
namespace ast {

NodeAsSourceCode AsSourceCode(const ast::Node& node) {
  return NodeAsSourceCode{&node};
}

std::ostream& operator<<(std::ostream& ostream,
                         const NodeAsSourceCode& printable) {
  const auto& node = *printable.node;
  return ostream << base::UTF16ToUTF8(node.range().GetString());
}

}  // namespace ast
}  // namespace joana
