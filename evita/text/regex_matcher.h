// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_regex_matcher_h)
#define INCLUDE_evita_text_regex_matcher_h

#include <memory>

#include "base/basictypes.h"
#include "evita/text/search_and_replace_model.h"
#include "regex/IRegex.h"

namespace text {

class RegexMatcher {
  private: class BufferMatchContext;

  public: struct ErrorInfo {
    int error_code;
    int offset;

    ErrorInfo() : error_code(0), offset(0) {
    }
  };

  private: ErrorInfo error_info_;
  private: bool matched_;
  private: std::unique_ptr<BufferMatchContext> match_context_;
  private: Regex::IRegex* regex_;
  private: SearchParameters search_params_;

  public: RegexMatcher(const text::SearchParameters* params,
                       text::Buffer* buffer, Posn start, Posn end);
  public: ~RegexMatcher();

  public: bool FirstMatch();
  public: ErrorInfo GetError() const;
  private: text::Range* GetMatched(const base::string16& name);
  public: text::Range* GetMatched(int);
  public: bool NextMatch();
  public: void Replace(const base::string16& with, bool meta_char);
  public: bool WrapMatch();

  DISALLOW_COPY_AND_ASSIGN(RegexMatcher);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_regex_matcher_h)
