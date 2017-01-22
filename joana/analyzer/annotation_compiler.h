// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANNOTATION_COMPILER_H_
#define JOANA_ANALYZER_ANNOTATION_COMPILER_H_

#include <vector>

#include "joana/analyzer/annotation.h"

#include "base/macros.h"
#include "base/optional.h"

namespace joana {
namespace analyzer {

class Context;
enum class ErrorCode;

//
// Annotation::Compiler
//
class Annotation::Compiler final {
 public:
  // When |context| is |nullptr|, error is not reported.
  explicit Compiler(const base::Optional<Context*>& maybe_context = {});
  Compiler();
  ~Compiler();

  Annotation Compile(const ast::Node& document, const ast::Node& target);

 private:
  void AddError(const ast::Node& node,
                ErrorCode error_code,
                const ast::Node& related);

  void AddError(const ast::Node& document, ErrorCode error_code);

  void Classify(const ast::Node& document);
  void ProcessTemplateTag(const ast::Node& node);
  void RememberType(const ast::Node& node);
  void RememberTag(const ast::Node** pointer, const ast::Node& node);

  // Report |error_code| error for |tag| if it is not |nullptr|.
  void ReportErrorTag(ErrorCode error_code, const ast::Node* maybe_tag);

  // Report |error_code| error for |tags|.
  void ReportErrorTags(ErrorCode error_code,
                       const std::vector<const ast::Node*>& tags);

  void Validate(const ast::Node& target);
  void ValidateForConstructor(const ast::Node& target);
  void ValidateForEnum(const ast::Node& target);
  void ValidateForFunction(const ast::Node& target);
  void ValidateForInterface(const ast::Node& target);
  void ValidateForType(const ast::Node& target);

  Annotation annotation_;
  const base::Optional<Context*> maybe_context_;

  DISALLOW_COPY_AND_ASSIGN(Compiler);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANNOTATION_COMPILER_H_
