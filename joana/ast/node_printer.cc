// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_printer.h"

#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/node.h"
#include "joana/base/escaped_string_piece.h"

namespace joana {
namespace ast {

NodeAsSourceCode AsSourceCode(const ast::Node& node, char delimiter) {
  return NodeAsSourceCode{delimiter, &node};
}

std::ostream& operator<<(std::ostream& ostream,
                         const NodeAsSourceCode& printable) {
  const auto& node = *printable.node;
  const auto& range = node.range();
  const auto delimiter = printable.delimiter;
  if (delimiter == 0)
    return ostream << base::UTF16ToUTF8(range.GetString());
  return ostream << EscapedStringPiece16(range.GetString(), delimiter);
}

}  // namespace ast
}  // namespace joana
