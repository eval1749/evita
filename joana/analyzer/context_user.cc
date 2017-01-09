// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/context_user.h"

#include "base/logging.h"
#include "joana/analyzer/context.h"

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
