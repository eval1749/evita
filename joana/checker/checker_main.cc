// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/parser/public/parse.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/edit_context_builder.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/error_sink.h"
#include "joana/public/memory/zone.h"
#include "joana/public/memory/zone_allocated.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_range.h"

namespace joana {
namespace internal {

class ErrorRecord final : public ZoneAllocated {
 public:
  ErrorRecord(const SourceCodeRange& range, int error_code);
  ~ErrorRecord();

  int error_code() const { return error_code_; }
  const SourceCodeRange& range() const { return range_; }

 private:
  const int error_code_;
  const SourceCodeRange range_;

  DISALLOW_COPY_AND_ASSIGN(ErrorRecord);
};

ErrorRecord::ErrorRecord(const SourceCodeRange& range, int error_code)
    : error_code_(error_code), range_(range) {}

ErrorRecord::~ErrorRecord() = default;

class SimpleErrorSink final : public ErrorSink {
 public:
  SimpleErrorSink();
  ~SimpleErrorSink();

  const std::vector<ErrorRecord*>& errors() const { return errors_; }

 private:
  // |ErrorSink| members
  void AddError(const SourceCodeRange& range, int error_code) final;

  Zone zone_;
  std::vector<ErrorRecord*> errors_;

  DISALLOW_COPY_AND_ASSIGN(SimpleErrorSink);
};

void SimpleErrorSink::AddError(const SourceCodeRange& range, int error_code) {
  auto* const record = new (&zone_) ErrorRecord(range, error_code);
  errors_.push_back(record);
}

SimpleErrorSink::SimpleErrorSink() : zone_("SimpleErrorSink") {}
SimpleErrorSink::~SimpleErrorSink() = default;

class Checker final {
 public:
  Checker();
  ~Checker() = default;

  void AddSourceCode(const base::FilePath& file_path,
                     base::StringPiece16 file_contents);

  int Run();

 private:
  SimpleErrorSink error_sink_;
  Zone node_zone_;
  ast::NodeFactory node_factory_;
  std::unique_ptr<ast::EditContext> context_;
  std::vector<const SourceCode*> source_codes_;
  Zone source_code_zone_;
  SourceCode::Factory source_code_factory_;
  std::unordered_map<const SourceCode*, const ast::Node*> ast_map_;

  DISALLOW_COPY_AND_ASSIGN(Checker);
};

Checker::Checker()
    : node_zone_("Checker.Node"),
      node_factory_(&node_zone_),
      context_(ast::EditContext::Builder()
                   .SetErrorSink(&error_sink_)
                   .SetNodeFactory(&node_factory_)
                   .Build()),
      source_code_zone_("Checker.SourceCode"),
      source_code_factory_(&source_code_zone_) {}

void Checker::AddSourceCode(const base::FilePath& file_path,
                            base::StringPiece16 file_contents) {
  const auto& source_code = source_code_factory_.New(file_path, file_contents);
  source_codes_.push_back(&source_code);
  const auto& module = Parse(context_.get(), source_code.range());
  ast_map_.emplace(&source_code, &module);
}

int Checker::Run() {
  for (auto* const error : error_sink_.errors()) {
    std::cerr << error->range().source_code().file_path().LossyDisplayName()
              << '(' << error->range().start() << ':' << error->range().end()
              << ':' << error->error_code() << std::endl;
  }
  return error_sink_.errors().size() == 0 ? 0 : 1;
}

}  // namespace internal
}  // namespace joana

extern "C" int main() {
  base::AtExitManager at_exit;
#if defined(OS_WIN)
  base::CommandLine::set_slash_is_not_a_switch();
#endif
  base::CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }

  joana::internal::Checker checker;
  const auto* const command_line = base::CommandLine::ForCurrentProcess();
  for (const auto& file_name : command_line->GetArgs()) {
    base::FilePath file_path =
        base::MakeAbsoluteFilePath(base::FilePath(file_name));
    std::string file_contents8;
    if (!base::ReadFileToString(file_path, &file_contents8)) {
      LOG(ERROR) << "Cannot read file " << file_path.value();
      continue;
    }
    VLOG(0) << "Process " << file_path.value();
    const auto& file_contents = base::UTF8ToUTF16(file_contents8);
    checker.AddSourceCode(file_path, base::StringPiece16(file_contents));
  }
  return checker.Run();
}
