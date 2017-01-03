// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_COMPILATION_UNITS_H_
#define JOANA_AST_COMPILATION_UNITS_H_

#include <unordered_map>

#include "joana/ast/syntax.h"
#include "joana/base/memory/zone_unordered_map.h"

namespace joana {
namespace ast {

//
// CompilationUnit
//
class JOANA_AST_EXPORT CompilationUnit : public SyntaxTemplate<Syntax> {
  DECLARE_ABSTRACT_AST_SYNTAX(CompilationUnit, Syntax);

 public:
  ~CompilationUnit() override;

 protected:
  explicit CompilationUnit(SyntaxCode syntax_code);

 private:
  DISALLOW_COPY_AND_ASSIGN(CompilationUnit);
};

//
// ExternsSyntax
//
class JOANA_AST_EXPORT ExternsSyntax final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Externs, CompilationUnit);

 public:
  ~ExternsSyntax() final;

 private:
  ExternsSyntax();

  DISALLOW_COPY_AND_ASSIGN(ExternsSyntax);
};

//
// ModuleSyntax
//
class JOANA_AST_EXPORT ModuleSyntax final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Module, CompilationUnit);

 public:
  ~ModuleSyntax() final;

 private:
  ModuleSyntax();

  DISALLOW_COPY_AND_ASSIGN(ModuleSyntax);
};

//
// ScriptSyntax
//
class JOANA_AST_EXPORT ScriptSyntax final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Script, CompilationUnit);

 public:
  ~ScriptSyntax() final;

 private:
  ScriptSyntax();

  DISALLOW_COPY_AND_ASSIGN(ScriptSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_COMPILATION_UNITS_H_
