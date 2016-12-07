// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_EDIT_CONTEXT_H_
#define JOANA_PUBLIC_AST_EDIT_CONTEXT_H_

#include "base/macros.h"
#include "joana/public/public_export.h"

namespace joana {

class ErrorSink;

namespace ast {

class NodeFactory;

class JOANA_PUBLIC_EXPORT EditContext {
 public:
  class Builder;

  EditContext(ErrorSink* error_sink, NodeFactory* node_factory);
  ~EditContext();

  ErrorSink& error_sink() const { return *error_sink_; }
  NodeFactory& node_factory() const { return *node_factory_; }

 private:
  ErrorSink* const error_sink_;
  NodeFactory* const node_factory_;

  DISALLOW_COPY_AND_ASSIGN(EditContext);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_EDIT_CONTEXT_H_
