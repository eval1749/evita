// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/properties.h"

#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/syntax.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Properties
//
Properties::Properties(Zone* zone, const ast::Node& owner)
    : computed_name_map_(zone), name_map_(zone), owner_(owner) {}

Properties::~Properties() = default;

Property& Properties::Add(Property* property) {
  if (property->key() == ast::SyntaxCode::Name) {
    const auto& result =
        name_map_.emplace(ast::Name::IdOf(property->key()), property);
    DCHECK(result.second);
    return *property;
  }
  const auto& string_key = property->key().range().GetString();
  const auto& result = computed_name_map_.emplace(string_key, property);
  DCHECK(result.second);
  return *property;
}

Property& Properties::Get(const ast::Node& key) const {
  auto* property = TryGet(key);
  DCHECK(property) << key;
  return *property;
}

Property* Properties::TryGet(const ast::Node& key) const {
  if (key == ast::SyntaxCode::Name) {
    const auto& it = name_map_.find(ast::Name::IdOf(key));
    return it == name_map_.end() ? nullptr : it->second;
  }
  const auto& string_key = key.range().GetString();
  const auto& it = computed_name_map_.find(string_key);
  return it == computed_name_map_.end() ? nullptr : it->second;
}

}  // namespace analyzer
}  // namespace joana
