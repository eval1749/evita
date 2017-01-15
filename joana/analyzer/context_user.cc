// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/context_user.h"

#include "base/logging.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/type_factory.h"

namespace joana {
namespace analyzer {

//
// ContextUser
//
ContextUser::ContextUser(Context* context) : context_(*context) {
  DCHECK(context);
}

ContextUser::~ContextUser() = default;

Factory& ContextUser::factory() const {
  return context_.factory();
}

TypeFactory& ContextUser::type_factory() const {
  return context_.type_factory();
}

// |TypeFactory| shortcut
const Type& ContextUser::any_type() const {
  return type_factory().any_type();
}

const Type& ContextUser::invalid_type() const {
  return type_factory().invalid_type();
}

const Type& ContextUser::nil_type() const {
  return type_factory().nil_type();
}

const Type& ContextUser::null_type() const {
  return type_factory().null_type();
}

const Type& ContextUser::unspecified_type() const {
  return type_factory().unspecified_type();
}

const Type& ContextUser::void_type() const {
  return type_factory().void_type();
}

void ContextUser::AddError(const ast::Node& node,
                           ErrorCode error_code,
                           const ast::Node& related) {
  context_.AddError(node, error_code, related);
}

void ContextUser::AddError(const ast::Node& node, ErrorCode error_code) {
  context_.AddError(node, error_code);
}

void ContextUser::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  context_.AddError(range, error_code);
}

}  // namespace analyzer
}  // namespace joana
