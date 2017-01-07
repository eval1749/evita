// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/context.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/public/analyzer_settings.h"
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
      settings_(settings) {}

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

Value* Context::TryValueOf(const ast::Node& node) const {
  return factory_->TryValueOf(node);
}

}  // namespace analyzer
}  // namespace joana
