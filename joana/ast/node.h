// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_H_
#define JOANA_AST_NODE_H_

#include <array>
#include <iosfwd>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "base/logging.h"
#include "base/macros.h"
#include "joana/ast/ast_export.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/memory/zone_vector.h"
#include "joana/base/source_code_range.h"

namespace joana {
namespace ast {

enum class TokenKind;
enum class TokenKind;
class Syntax;
enum class SyntaxCode;

class JOANA_AST_EXPORT Node final {
 public:
  ~Node();

  void* operator new(size_t size, void* pointer) { return pointer; }

  bool operator==(const Node& other) const;
  bool operator==(const Node* other) const;
  bool operator!=(const Node& other) const;
  bool operator!=(const Node* other) const;
  bool operator==(TokenKind kind) const;
  bool operator!=(TokenKind kind) const;
  bool operator==(SyntaxCode syntax_code) const;
  bool operator!=(SyntaxCode syntax_code) const;

  size_t arity() const { return arity_; }
  const Node& child_at(size_t index) const;
  const SourceCode& source_code() const { return range_.source_code(); }
  const SourceCodeRange& range() const { return range_; }
  const Syntax& syntax() const { return syntax_; }

  bool is_literal() const;

  // Operator helper functions
  int name_id() const;

 protected:
  Node(const SourceCodeRange& range, const Syntax& syntax, size_t arity);

 private:
  friend class NodeFactory;

  const size_t arity_;
  const SourceCodeRange range_;
  const Syntax& syntax_;
  const Node* nodes_[1];

  DISALLOW_COPY_AND_ASSIGN(Node);
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Node& node);
JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Node* node);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_H_
