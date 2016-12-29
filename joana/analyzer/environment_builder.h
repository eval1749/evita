// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include "joana/analyzer/pass.h"

namespace joana {

namespace ast {
class BindingElement;
class Declaration;
class Expression;
class Name;
class Node;
class Statement;
class VariableDeclaration;
}

namespace analyzer {

class Context;
class Environment;
enum class VariableKind;

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

  void BindToVariable(const ast::Node& assignment, const ast::Node& name);

  // Process AST nodes
  void ProcessBindingElement(const ast::Node& assignment,
                             const ast::BindingElement& element);

  void ProcessDeclaration(const ast::Declaration& declaration);

  void ProcessStatement(const ast::Statement& statement);

  void ProcessVariables(const ast::VariableDeclaration& declaration);

  // The current processing environment
  Environment* environment_;

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
