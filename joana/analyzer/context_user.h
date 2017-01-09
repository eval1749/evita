// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_CONTEXT_USER_H_
#define JOANA_ANALYZER_CONTEXT_USER_H_

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

class SourceCodeRange;

namespace analyzer {

class Context;
enum class ErrorCode;
class Factory;
class TypeFactory;

//
// ContextUser
//
class ContextUser {
 public:
  ~ContextUser();

 protected:
  explicit ContextUser(Context* context);

  const Context& context() const { return context_; }
  Context& context() { return context_; }
  Factory& factory() const;
  TypeFactory& type_factory() const;

  void AddError(const ast::Node& node, ErrorCode error_code);
  void AddError(const ast::Node& node,
                ErrorCode error_code,
                const ast::Node& related);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);

 private:
  Context& context_;

  DISALLOW_COPY_AND_ASSIGN(ContextUser);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CONTEXT_USER_H_
