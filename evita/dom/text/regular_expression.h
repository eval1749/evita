// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_REGULAR_EXPRESSION_H_
#define EVITA_DOM_TEXT_REGULAR_EXPRESSION_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Document;
class RegExpInit;

namespace bindings {
class RegularExpressionClass;
}

//////////////////////////////////////////////////////////////////////
//
// RegularExpression
//
class RegularExpression final : public v8_glue::Scriptable<RegularExpression> {
  DECLARE_SCRIPTABLE_OBJECT(RegularExpression);

 public:
  ~RegularExpression() final;

  v8::Handle<v8::Value> ExecuteOnDocument(Document* document,
                                          int start,
                                          int end);

 private:
  friend class bindings::RegularExpressionClass;
  friend class BufferMatcher;
  friend class RegularExpressionCompiler;
  class BufferMatcher;
  class Compiler;
  struct Match;
  class RegularExpressionImpl;

  RegularExpression(RegularExpressionImpl* regex,
         const base::string16& source,
         const RegExpInit& init_dict);

  bool backward() const { return backward_; }
  bool global() const { return global_; }
  bool ignore_case() const { return ignore_case_; }
  bool match_exact() const { return match_exact_; }
  bool match_word() const { return match_word_; }
  bool multiline() const { return multiline_; }
  const base::string16& source() const { return source_; }
  bool sticky() const { return sticky_; }

  v8::Local<v8::Value> MakeMatchArray(const std::vector<Match>& matchs);

  static RegularExpression* NewRegularExpression(const base::string16& source,
                           const RegExpInit& options);
  static RegularExpression* NewRegularExpression(const base::string16& source);

  bool backward_;
  bool global_;
  bool ignore_case_;
  bool match_exact_;
  bool match_word_;
  bool multiline_;
  std::unique_ptr<RegularExpressionImpl> regex_;
  base::string16 source_;
  bool sticky_;

  DISALLOW_COPY_AND_ASSIGN(RegularExpression);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_REGULAR_EXPRESSION_H_
