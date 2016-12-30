// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_COMPILATION_UNITS_H_
#define JOANA_AST_COMPILATION_UNITS_H_

#include <unordered_map>

#include "joana/ast/node.h"
#include "joana/base/memory/zone_unordered_map.h"

namespace joana {
namespace ast {

class JsDoc;
class StatementList;

//
// CompilationUnit
//
class JOANA_AST_EXPORT CompilationUnit
    : public NodeTemplate<Node, const StatementList*> {
  DECLARE_ABSTRACT_AST_NODE(CompilationUnit, Node);

 public:
  ~CompilationUnit() override;

  const StatementList& statements() const { return *member_at<0>(); }

  const JsDoc* JsDocFor(const ast::Node& node) const;

 protected:
  CompilationUnit(
      Zone* zone,
      const SourceCodeRange& range,
      const StatementList& statements,
      const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map);

 private:
  const ZoneUnorderedMap<const Node*, const ast::JsDoc*> jsdoc_map_;

  DISALLOW_COPY_AND_ASSIGN(CompilationUnit);
};

//
// Externs
//
class JOANA_AST_EXPORT Externs final : public CompilationUnit {
  DECLARE_CONCRETE_AST_NODE(Externs, CompilationUnit);

 public:
  ~Externs() final;

 private:
  Externs(Zone* zone,
          const SourceCodeRange& range,
          const StatementList& statements,
          const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map);

  DISALLOW_COPY_AND_ASSIGN(Externs);
};

//
// Module
//
class JOANA_AST_EXPORT Module final : public CompilationUnit {
  DECLARE_CONCRETE_AST_NODE(Module, CompilationUnit);

 public:
  ~Module() final;

 private:
  Module(Zone* zone,
         const SourceCodeRange& range,
         const StatementList& statements,
         const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map);

  DISALLOW_COPY_AND_ASSIGN(Module);
};

//
// Script
//
class JOANA_AST_EXPORT Script final : public CompilationUnit {
  DECLARE_CONCRETE_AST_NODE(Script, CompilationUnit);

 public:
  ~Script() final;

 private:
  Script(Zone* zone,
         const SourceCodeRange& range,
         const StatementList& statements,
         const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map);

  DISALLOW_COPY_AND_ASSIGN(Script);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_COMPILATION_UNITS_H_
