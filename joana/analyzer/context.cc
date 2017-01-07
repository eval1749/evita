// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/analyzer/context.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/public/analyzer_settings.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/value_map.h"
#include "joana/analyzer/values.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"

namespace joana {
namespace analyzer {

//
// Context
//
Context::Context(const AnalyzerSettings& settings)
    : factory_(new Factory(&settings.zone())),
      global_environment_(NewGlobalEnvironment(&settings.zone())),
      settings_(settings),
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
  global_environment_.Bind(name, &variable);
}

void Context::InstallPrimitiveTypes() {
  static const ast::TokenKind kTypeNames[] = {
      ast::TokenKind::Boolean, ast::TokenKind::Null,
      ast::TokenKind::Number,  ast::TokenKind::String,
      ast::TokenKind::Symbol,  ast::TokenKind::Undefined,
      ast::TokenKind::Void,
  };
  for (const auto id : BuiltInWorld::GetInstance()->primitive_types()) {
    const auto& name = BuiltInWorld::GetInstance()->NameOf(id);
    const auto& type = BuiltInWorld::GetInstance()->TypeOf(id);
    RegisterValue(type, &factory().NewPrimitiveType(name));
    auto& variable = factory().NewVariable(name);
    Value::Editor().AddAssignment(&variable, type);
    global_environment_.Bind(name, &variable);
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

Value& Context::RegisterValue(const ast::Node& node, Value* value) {
  return value_map_->RegisterValue(node, value);
}

Value* Context::TryValueOf(const ast::Node& node) const {
  return value_map_->TryValueOf(node);
}

Value& Context::ValueOf(const ast::Node& node) const {
  return value_map_->ValueOf(node);
}

}  // namespace analyzer
}  // namespace joana
