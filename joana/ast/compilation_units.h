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
// Externs
//
class JOANA_AST_EXPORT Externs final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Externs, CompilationUnit);

 public:
  ~Externs() final;

 private:
  Externs();

  DISALLOW_COPY_AND_ASSIGN(Externs);
};

//
// Module
//
class JOANA_AST_EXPORT Module final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Module, CompilationUnit);

 public:
  ~Module() final;

 private:
  Module();

  DISALLOW_COPY_AND_ASSIGN(Module);
};

//
// Script
//
class JOANA_AST_EXPORT Script final : public CompilationUnit {
  DECLARE_CONCRETE_AST_SYNTAX(Script, CompilationUnit);

 public:
  ~Script() final;

 private:
  Script();

  DISALLOW_COPY_AND_ASSIGN(Script);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_COMPILATION_UNITS_H_
