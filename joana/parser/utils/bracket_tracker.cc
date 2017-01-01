// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "joana/parser/utils/bracket_tracker.h"

#include "joana/ast/node.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"

namespace joana {
namespace parser {

namespace {

std::pair<ast::PunctuatorKind, ast::PunctuatorKind> ComputeMinMax(
    const std::vector<BracketTracker::Description>& descriptions) {
  auto max = descriptions.front().open;
  auto min = max;
  for (const auto& description : descriptions) {
    min = std::min(min, description.open);
    min = std::min(min, description.close);
    max = std::max(max, description.open);
    max = std::max(max, description.close);
  }
  return std::make_pair(min, max);
}

}  // namespace

//
// BracketTracker
//
BracketTracker::BracketTracker(ErrorSink* error_sink,
                               const SourceCodeRange& range,
                               const std::vector<Description>& descriptions)
    : descriptions_(descriptions),
      error_sink_(*error_sink),
      min_max_(ComputeMinMax(descriptions)),
      source_code_range_(range) {}

BracketTracker::~BracketTracker() = default;

ast::PunctuatorKind BracketTracker::close_bracket() const {
  DCHECK(!stack_.empty());
  return stack_.top().second->close;
}

void BracketTracker::Check(const ast::Node& token, const Description& actual) {
  DCHECK_EQ(token, actual.close);
  if (stack_.empty()) {
    error_sink_.AddError(token.range(), actual.close_error);
    return;
  }
  const auto& open_token = *stack_.top().first;
  if (open_token == actual.open) {
    stack_.pop();
    return;
  }

  // We get mismatched close bracket.
  error_sink_.AddError(
      SourceCodeRange::Merge(token.range(), open_token.range()),
      actual.close_error);
}

void BracketTracker::Mark(const ast::Node& token,
                          const Description& description) {
  stack_.push(std::make_pair(&token, &description));
}

void BracketTracker::Feed(const ast::Node& token) {
  if (token != ast::SyntaxCode::Punctuator)
    return;
  const auto kind = token.syntax().As<ast::PunctuatorSyntax>().kind();
  if (kind < min_max_.first || kind > min_max_.second)
    return;
  for (const auto& description : descriptions_) {
    if (kind == description.open)
      return Mark(token, description);
    if (kind == description.close)
      return Check(token, description);
  }
}

void BracketTracker::Finish() {
  if (stack_.empty())
    return;
  // Report the last open bracket pair.
  const auto& open_bracket = *stack_.top().first;
  const auto& description = *stack_.top().second;
  error_sink_.AddError(
      source_code_range_.source_code().Slice(open_bracket.range().start(),
                                             source_code_range_.end()),
      description.open_error);
}

}  // namespace parser
}  // namespace joana
