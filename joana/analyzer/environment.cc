// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/environment.h"

#include "base/logging.h"
#include "joana/ast/node_forward.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Environment
//
Environment::Environment(Environment* outer, const ast::Node& owner)
    : outer_(outer), owner_(owner) {}

Environment::Environment(const ast::Node& owner)
    : Environment(nullptr, owner) {}

Environment::~Environment() = default;

void Environment::Bind(const ast::Name& name, Value* value) {
  const auto& result = value_map_.emplace(name.number(), value);
  DCHECK(result.second) << name << " is already bound.";
  const auto& name_result = name_map_.emplace(name.number(), &name);
  DCHECK(name_result.second);
}

Value* Environment::BindingOf(const ast::Name& name) const {
  const auto& it = value_map_.find(name.number());
  return it == value_map_.end() ? nullptr : it->second;
}

}  // namespace analyzer
}  // namespace joana
