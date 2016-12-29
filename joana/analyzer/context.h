// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_CONTEXT_H_
#define JOANA_ANALYZER_CONTEXT_H_

#include <memory>

#include "base/macros.h"

namespace joana {

class AnalyzerSettings;
class SourceCodeRange;

namespace analyzer {

class Factory;
enum class ErrorCode;

//
// Context
//
class Context final {
 public:
  explicit Context(const AnalyzerSettings& settings);
  ~Context();

  Factory& factory() const { return *factory_; }

  void AddError(const SourceCodeRange& range, ErrorCode error_code);

 private:
  std::unique_ptr<Factory> factory_;
  const AnalyzerSettings& settings_;

  DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CONTEXT_H_
