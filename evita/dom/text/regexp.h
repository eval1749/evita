// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_REGEXP_H_
#define EVITA_DOM_TEXT_REGEXP_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Document;
class RegExpInit;

namespace bindings {
class RegExpClass;
}

//////////////////////////////////////////////////////////////////////
//
// RegExp
//
class RegExp final : public v8_glue::Scriptable<RegExp> {
  DECLARE_SCRIPTABLE_OBJECT(RegExp);

 public:
  ~RegExp() final;

  v8::Handle<v8::Value> ExecuteOnDocument(Document* document,
                                          int start,
                                          int end);

 private:
  friend class bindings::RegExpClass;
  friend class BufferMatcher;
  friend class RegExpCompiler;
  class BufferMatcher;
  class Compiler;
  struct Match;
  class RegExpImpl;

  RegExp(RegExpImpl* regex,
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

  static RegExp* NewRegExp(const base::string16& source,
                           const RegExpInit& options);
  static RegExp* NewRegExp(const base::string16& source);

  bool backward_;
  bool global_;
  bool ignore_case_;
  bool match_exact_;
  bool match_word_;
  bool multiline_;
  std::unique_ptr<RegExpImpl> regex_;
  base::string16 source_;
  bool sticky_;

  DISALLOW_COPY_AND_ASSIGN(RegExp);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_REGEXP_H_
