// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/edit_context.h"

namespace joana {
namespace ast {

EditContext::EditContext(ErrorSink* error_sink, NodeFactory* node_factory)
    : error_sink_(error_sink), node_factory_(node_factory) {}

EditContext::~EditContext() = default;

}  // namespace ast
}  // namespace joana
