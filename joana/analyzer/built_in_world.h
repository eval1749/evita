// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_BUILT_IN_WORLD_H_
#define JOANA_ANALYZER_BUILT_IN_WORLD_H_

#include <memory>

#include "base/macros.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace joana {
namespace ast {
class Node;
enum class TokenKind;
}
namespace analyzer {

//
// BuiltInWorld
//
class BuiltInWorld final {
 public:
  ~BuiltInWorld();

  const ast::Node& global_module() const { return global_module_; }

  const ast::Node& NameOf(ast::TokenKind kind) const;
  const ast::Node& TypeOf(ast::TokenKind kind) const;

  static BuiltInWorld* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<BuiltInWorld>;
  class Private;

  BuiltInWorld();

  std::unique_ptr<Private> private_;
  const ast::Node& global_module_;

  DISALLOW_COPY_AND_ASSIGN(BuiltInWorld);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_BUILT_IN_WORLD_H_
