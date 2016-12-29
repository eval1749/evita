// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
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
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/analyzer/public/analyze.h"
#include "joana/analyzer/public/analyzer_settings.h"
#include "joana/analyzer/public/analyzer_settings_builder.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/node_factory.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/base/source_code_line.h"
#include "joana/base/source_code_line_cache.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parse.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/public/parser_options_builder.h"

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

//
// Module
//
class ScriptModule final {
 public:
  ScriptModule(const SourceCode& source_code, const ast::Node& root_node);
  ScriptModule() = default;

  SourceCodeLine SourceCodeLinetAt(int offset) const;

 private:
  const std::unique_ptr<SourceCodeLine::Cache> line_cache_;
  const ast::Node& root_node_;
  const SourceCode& source_code_;

  DISALLOW_COPY_AND_ASSIGN(ScriptModule);
};

ScriptModule::ScriptModule(const SourceCode& source_code,
                           const ast::Node& root_node)
    : line_cache_(new SourceCodeLine::Cache(source_code)),
      root_node_(root_node),
      source_code_(source_code) {}

SourceCodeLine ScriptModule::SourceCodeLinetAt(int offset) const {
  return line_cache_->Get(offset);
}

void PrintSourceCodeLine(int start, int end, const SourceCodeRange& range) {
  for (const auto& ch :
       base::UTF16ToUTF8(range.source_code().GetString(start, end))) {
    if (ch == '\t') {
      std::cout << ' ';
      continue;
    }
    if (ch == '\n')
      break;
    std::cout << ch;
  }
  std::cout << std::endl;
  const auto stop = std::min(end, range.end());
  if (range.IsCollapsed()) {
    for (auto runner = start; runner < stop; ++runner)
      std::cout << ' ';
    std::cout << '^' << std::endl;
    return;
  }
  for (auto runner = start; runner < stop; ++runner)
    std::cout << (range.Contains(runner) ? '~' : ' ');
  std::cout << std::endl;
}

void PrintSourceCodeRange(const SourceCodeLine& start_line,
                          const SourceCodeLine& end_line,
                          const SourceCodeRange& range) {
  const auto kBeforeContext = 20;
  const auto kAfterContext = 40;
  const auto kLineWidth = 80;

  if (start_line == end_line && start_line.size() <= kLineWidth) {
    PrintSourceCodeLine(start_line.start(), start_line.end(), range);
    return;
  }

  const auto start_line_start =
      std::max(start_line.start(), range.start() - kBeforeContext);
  const auto start_line_end =
      std::min(start_line_start + kLineWidth, start_line.end());
  PrintSourceCodeLine(start_line_start, start_line_end, range);
  if (range.end() <= start_line_end)
    return;
  if (start_line.end() != end_line.start())
    std::cout << "  ...." << std::endl;
  PrintSourceCodeLine(std::max(end_line.start(), range.end() - kBeforeContext),
                      std::min(end_line.end(), range.end() + kAfterContext),
                      range);
}

//
// Checker
//
class Checker final {
 public:
  static int Main();

 private:
  explicit Checker(const ParserOptions& options);
  ~Checker() = default;

  void AddSourceCode(const base::FilePath& file_path,
                     base::StringPiece16 file_contents);

  void Analyze();

  ScriptModule& ModuleOf(const SourceCode& source_code) const;
  int Run();

  SimpleErrorSink error_sink_;
  Zone node_zone_;
  ast::NodeFactory node_factory_;
  const std::unique_ptr<ParserContext> context_;
  std::vector<const ast::Node*> modules_;
  std::unordered_map<const SourceCode*, std::unique_ptr<ScriptModule>>
      module_map_;
  std::vector<const SourceCode*> source_codes_;
  Zone source_code_zone_;
  SourceCode::Factory source_code_factory_;
  ParserOptions options_;

  DISALLOW_COPY_AND_ASSIGN(Checker);
};

Checker::Checker(const ParserOptions& options)
    : node_zone_("Checker.Node"),
      node_factory_(&node_zone_),
      context_(ParserContext::Builder()
                   .set_error_sink(&error_sink_)
                   .set_node_factory(&node_factory_)
                   .Build()),
      source_code_zone_("Checker.SourceCode"),
      source_code_factory_(&source_code_zone_),
      options_(options) {}

void Checker::AddSourceCode(const base::FilePath& file_path,
                            base::StringPiece16 file_contents) {
  const auto& source_code = source_code_factory_.New(file_path, file_contents);
  source_codes_.push_back(&source_code);
  const auto& module = Parse(context_.get(), source_code.range(), options_);
  modules_.push_back(&module);
  module_map_.emplace(&source_code, new ScriptModule(source_code, module));
}

// Analyze modules after we parse all modules.
void Checker::Analyze() {
  Zone zone("AnalyzerSettings");
  const auto& settings = joana::AnalyzerSettings::Builder()
                             .set_error_sink(&error_sink_)
                             .set_zone(&zone)
                             .Build();
  Analyzer analyzer(*settings);
  for (const auto& module : modules_)
    analyzer.Load(*module);
  analyzer.Analyze();
}

int Checker::Main() {
  auto* const command_line = base::CommandLine::ForCurrentProcess();

  {
    const auto& file_path = command_line->GetSwitchValuePath("input");
    if (!file_path.empty()) {
      const auto& abs_file_path = base::MakeAbsoluteFilePath(file_path);
      std::string string8;
      if (!base::ReadFileToString(abs_file_path, &string8)) {
        std::cout << "Failed to read " << file_path.value();
        return EXIT_FAILURE;
      }
      const auto& inputs = base::SplitString(
          string8, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
      for (const auto& input : inputs)
        command_line->AppendArg(input);
    }
  }

  const auto& options =
      ParserOptions::Builder()
          .set_disable_automatic_semicolon(
              command_line->HasSwitch("disable_automatic_semicolon"))
          .set_enable_strict_backslash(
              command_line->HasSwitch("enable_strict_backslash"))
          .set_enable_strict_regexp(
              command_line->HasSwitch("enable_strict_regexp"))
          .Build();

  Checker checker(options);
  for (const auto& file_name : command_line->GetArgs()) {
    const auto& file_path =
        base::MakeAbsoluteFilePath(base::FilePath(file_name));
    std::string file_contents8;
    if (!base::ReadFileToString(file_path, &file_contents8)) {
      LOG(ERROR) << "Cannot read file " << file_path.value();
      continue;
    }
    VLOG(0) << "Process " << file_path.value();
    if (file_contents8.find("\n<include ") != std::string::npos) {
      VLOG(0) << "Ignore " << file_path.value() << " contains <include>";
      continue;
    }
    if (file_contents8.find("\n<if ") != std::string::npos) {
      VLOG(0) << "Ignore " << file_path.value() << " contains <include>";
      continue;
    }
    const auto& file_contents = base::UTF8ToUTF16(file_contents8);
    checker.AddSourceCode(file_path, base::StringPiece16(file_contents));
  }
  return checker.Run();
}

int Checker::Run() {
  Analyze();

  for (auto* const error : error_sink_.errors()) {
    const auto& source_code = error->range().source_code();
    const auto& module = ModuleOf(source_code);
    const auto& start_line = module.SourceCodeLinetAt(error->range().start());
    const auto& end_line = module.SourceCodeLinetAt(error->range().end());
    std::cout << source_code.file_path().value() << '(' << end_line.number()
              << ") " << ast::ErrorStringOf(error->error_code()) << std::endl;
    PrintSourceCodeRange(start_line, end_line, error->range());
  }
  return error_sink_.errors().size() == 0 ? 0 : 1;
}

ScriptModule& Checker::ModuleOf(const SourceCode& source_code) const {
  const auto& it = module_map_.find(&source_code);
  DCHECK(it != module_map_.end()) << source_code.file_path().value()
                                  << " is not found.";
  return *it->second;
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

  return joana::internal::Checker::Main();
}
