// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/analyzer/context.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/properties_editor.h"
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
      global_properties_(factory_->NewProperties(
          BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Global))),
      settings_(settings),
      type_factory_(new TypeFactory(&settings.zone())),
      type_map_(new TypeMap()),
      value_map_(new ValueMap()) {}

Context::~Context() = default;

ErrorSink& Context::error_sink() const {
  return settings_.error_sink();
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

// Global class
Class& Context::InstallClass(ast::TokenKind name_id) {
  const auto& object_name = BuiltInWorld::GetInstance()->NameOf(name_id);
  auto& object_class =
      factory().NewNormalClass(ClassKind::Class, object_name, object_name,
                               &factory().NewProperties(object_name));
  auto& object_variable = factory().NewVariable(
      VariableKind::Const, object_name, &factory().NewValueHolderData(),
      &factory().NewProperties(object_name));
  auto& object_property = factory().NewProperty(Visibility::Public, object_name,
                                                &object_variable.data(),
                                                &object_variable.properties());
  RegisterValue(object_name, &object_class);
  Value::Editor().AddAssignment(&object_property, object_name);
  Properties::Editor().Add(&global_properties(), &object_property);
  return object_class;
}

Class* Context::TryClassOf(ast::TokenKind name_id) const {
  const auto& object_name = BuiltInWorld::GetInstance()->NameOf(name_id);
  auto* object_property = global_properties().TryGet(object_name);
  if (object_property->assignments().size() != 1)
    return nullptr;
  const auto& assignment = *object_property->assignments().front();
  auto* object_value = TryValueOf(assignment);
  return object_value && object_value->Is<Class>() ? &object_value->As<Class>()
                                                   : nullptr;
}

}  // namespace analyzer
}  // namespace joana
