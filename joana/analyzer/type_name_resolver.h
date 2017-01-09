// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_NAME_RESOLVER_H_
#define JOANA_ANALYZER_TYPE_NAME_RESOLVER_H_

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Type;

//
// TypeNameResolver
//
class TypeNameResolver {
 public:
  ~TypeNameResolver();

  virtual const Type& ResolveTypeName(const ast::Node& node) = 0;

 protected:
  TypeNameResolver();

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeNameResolver);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_NAME_RESOLVER_H_
