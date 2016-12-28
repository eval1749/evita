// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include "base/macros.h"

namespace joana {
namespace ast {
class Declaration;
class Expression;
class Name;
class Node;
class Statement;
}
namespace analyzer {

class Environment;
class Factory;

//
// EnvironmentBuilder
//
class EnvironmentBuilder final {
 public:
  explicit EnvironmentBuilder(Factory* factory);
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

  Environment* environment_;
  Factory& factory_;

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
