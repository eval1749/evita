//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Regex Matcher
// listener/winapp/RegexMatcher.h
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/RegexMatch.h#2 $
//
#if !defined(INCLUDE_RegexMatcher_h)
#define INCLUDE_RegexMatcher_h

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

  public: RegexMatcher(const SearchParameters* params, text::Buffer* buffer,
      Posn start, Posn end);
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

#endif //!defined(INCLUDE_RegexMatcher_h)
