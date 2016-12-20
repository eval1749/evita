// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_EDIT_CONTEXT_BUILDER_H_
#define JOANA_AST_EDIT_CONTEXT_BUILDER_H_

#include <memory>

#include "joana/ast/edit_context.h"

namespace joana {
namespace ast {

class NodeFactory;

class JOANA_AST_EXPORT EditContext::Builder {
 public:
  Builder();
  ~Builder();

  std::unique_ptr<EditContext> Build();

  Builder& SetErrorSink(ErrorSink* error_sink);
  Builder& SetNodeFactory(NodeFactory* node_factory);

 private:
  ErrorSink* error_sink_ = nullptr;
  NodeFactory* node_factory_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EDIT_CONTEXT_BUILDER_H_
