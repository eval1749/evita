// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "joana/public/ast/edit_context_builder.h"

#include "base/logging.h"

namespace joana {
namespace ast {

using Builder = EditContext::Builder;

Builder::Builder() = default;
Builder::~Builder() = default;

std::unique_ptr<EditContext> Builder::Build() {
  DCHECK(error_sink_);
  DCHECK(node_factory_);
  return std::move(std::make_unique<EditContext>(error_sink_, node_factory_));
}

Builder& Builder::SetErrorSink(ErrorSink* error_sink) {
  DCHECK(error_sink);
  error_sink_ = error_sink;
  return *this;
}

Builder& Builder::SetNodeFactory(NodeFactory* node_factory) {
  DCHECK(node_factory);
  node_factory_ = node_factory;
  return *this;
}

}  // namespace ast
}  // namespace joana
