// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include "joana/analyzer/pass.h"

namespace joana {
namespace ast {
class Declaration;
class Expression;
class Name;
class Node;
class Statement;
}
namespace analyzer {

class Context;
class Environment;

//
// EnvironmentBuilder
//
class EnvironmentBuilder final : public Pass {
 public:
  explicit EnvironmentBuilder(Context* context);
  ~EnvironmentBuilder();

  void Load(const ast::Node& node);

 private:
  // Binding helpers
  void BindToFunction(const ast::Name& name,
                      const ast::Declaration& declaration);

  // Process AST nodes
  void ProcessDeclaration(const ast::Declaration& declaration);

  void ProcessStatement(const ast::Statement& statement);

  void ProcessVariable(const ast::Statement& statement,
                       const ast::Expression& expression);

  // The current processing environment
  Environment* environment_;

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
