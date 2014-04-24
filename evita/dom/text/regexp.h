// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_regex_h)
#define INCLUDE_evita_dom_regex_h

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
class RegExp : public v8_glue::Scriptable<RegExp> {
  DECLARE_SCRIPTABLE_OBJECT(RegExp);
  friend class bindings::RegExpClass;

  private: class BufferMatcher;
  private: class Compiler;
  private: struct Match;
  private: class RegExpImpl;

  friend class BufferMatcher;
  friend class RegExpCompiler;

  private: bool backward_;
  private: bool global_;
  private: bool ignore_case_;
  private: bool match_exact_;
  private: bool match_word_;
  private: bool multiline_;
  private: std::unique_ptr<RegExpImpl> regex_;
  private: base::string16 source_;
  private: bool sticky_;

  private: RegExp(RegExpImpl* regex, const base::string16& source,
                  const RegExpInit& init_dict);
  public: virtual ~RegExp();

  private: bool backward() const { return backward_; }
  private: bool global() const { return global_; }
  private: bool ignore_case() const { return ignore_case_; }
  private: bool match_exact() const { return match_exact_; }
  private: bool match_word() const { return match_word_; }
  private: bool multiline() const { return multiline_; }
  private: const base::string16& source() const { return source_; }
  private: bool sticky() const { return sticky_; }

  public: v8::Handle<v8::Value> ExecuteOnDocument(Document* document, int start,
                                                  int end);
  private: v8::Local<v8::Value> MakeMatchArray(
      const std::vector<Match>& matchs);

  private: static RegExp* NewRegExp(const base::string16& source,
                                    const RegExpInit& options);
  private: static RegExp* NewRegExp(const base::string16& source);

  DISALLOW_COPY_AND_ASSIGN(RegExp);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_regex_h)
