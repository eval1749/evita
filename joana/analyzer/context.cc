// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/analyzer/context.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/public/analyzer_settings.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/type_map.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/value_map.h"
#include "joana/analyzer/values.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"

namespace joana {
namespace analyzer {

//
// Context
//
Context::Context(const AnalyzerSettings& settings)
    : factory_(new Factory(&settings.zone())),
      global_environment_(NewGlobalEnvironment(&settings.zone())),
      settings_(settings),
      type_factory_(new TypeFactory(&settings.zone())),
      type_map_(new TypeMap()),
      value_map_(new ValueMap()) {
  InstallPrimitiveTypes();
  InstallGlobalObject();
  factory().ResetValueId();
}

Context::~Context() = default;

ErrorSink& Context::error_sink() const {
  return settings_.error_sink();
}

Environment& Context::global_environment() const {
  return global_environment_;
}

Zone& Context::zone() const {
  return settings_.zone();
}

void Context::AddError(const ast::Node& node,
                       ErrorCode error_code,
                       const ast::Node& related) {
  if (node.range().source_code() == related.range().source_code()) {
    AddError(SourceCodeRange::Merge(node.range(), related.range()), error_code);
    return;
  }
  AddError(related, error_code);
  AddError(node, error_code);
}

void Context::AddError(const ast::Node& node, ErrorCode error_code) {
  AddError(node.range(), error_code);
}

void Context::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  error_sink().AddError(range, error_code);
}

Environment& Context::EnvironmentOf(const ast::Node& node) const {
  const auto& it = environment_map_.find(&node);
  if (it != environment_map_.end())
    return *it->second;
  DCHECK(node.syntax().Is<ast::CompilationUnit>()) << node;
  return global_environment();
}

void Context::InstallGlobalObject() {
  const auto& name =
      BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Global);
  auto& variable = factory().NewVariable(name);
  Value::Editor().AddAssignment(&variable, name);
  global_environment_.BindVariable(name, &variable);
}

void Context::InstallPrimitiveTypes() {
  for (const auto id : BuiltInWorld::GetInstance()->primitive_types()) {
    const auto& name = BuiltInWorld::GetInstance()->NameOf(id);
    const auto& type = type_factory().GetPrimitiveType(id);
    RegisterType(name, type);
    global_environment_.BindType(name, type);
  }
}

Environment& Context::NewEnvironment(Environment* outer,
                                     const ast::Node& owner) {
  auto& environment = *new (&zone()) Environment(&zone(), outer, owner);
  const auto& result = environment_map_.emplace(&owner, &environment);
  DCHECK(result.second) << "Node can have only one environment " << owner;
  return environment;
}

// static
Environment& Context::NewGlobalEnvironment(Zone* zone) {
  const auto& module = BuiltInWorld::GetInstance()->global_module();
  return *new (zone) Environment(zone, module);
}

void Context::RegisterValue(const ast::Node& node, Value* value) {
  value_map_->RegisterValue(node, value);
}

Value* Context::TryValueOf(const ast::Node& node) const {
  return value_map_->TryValueOf(node);
}

Value& Context::ValueOf(const ast::Node& node) const {
  return value_map_->ValueOf(node);
}

void Context::RegisterType(const ast::Node& node, const Type& type) {
  type_map_->RegisterType(node, type);
}

const Type* Context::TryTypeOf(const ast::Node& node) const {
  return type_map_->TryTypeOf(node);
}

const Type& Context::TypeOf(const ast::Node& node) const {
  return type_map_->TypeOf(node);
}

}  // namespace analyzer
}  // namespace joana
