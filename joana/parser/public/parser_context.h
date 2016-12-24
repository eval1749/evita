// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSER_CONTEXT_H_
#define JOANA_PARSER_PUBLIC_PARSER_CONTEXT_H_

#include <memory>

#include "base/macros.h"
#include "joana/parser/public/parser_export.h"

namespace joana {

class ErrorSink;

namespace ast {
class NodeFactory;
}

//
// ParserContext
//
class JOANA_PARSER_EXPORT ParserContext final {
 public:
  JOANA_PARSER_EXPORT class Builder;

  ~ParserContext();

  ErrorSink& error_sink() const;
  ast::NodeFactory& node_factory() const;

 private:
  explicit ParserContext(const Builder& builder);

  ErrorSink& error_sink_;
  ast::NodeFactory& node_factory_;

  DISALLOW_COPY_AND_ASSIGN(ParserContext);
};

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSER_CONTEXT_H_
