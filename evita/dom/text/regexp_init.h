// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_regexp_init_h)
#define INCLUDE_evita_dom_regexp_init_h

#include "evita/dom/dictionary.h"

namespace dom {

class RegExpInit : public Dictionary {
  private: bool backward_;
  private: bool global_;
  private: bool ignore_case_;
  private: bool match_exact_;
  private: bool match_word_;
  private: bool multiline_;
  private: bool sticky_;

  public: RegExpInit();
  public: virtual ~RegExpInit();

  public: bool backward() const { return backward_; }
  public: void set_backward(bool backward) { backward_ = backward; }
  public: bool global() const { return global_; }
  public: void set_global(bool global) { global_ = global; }
  public: bool ignore_case() const { return ignore_case_; }
  public: void set_ignore_case(bool ignore_case) { ignore_case_ = ignore_case; }
  public: bool match_exact() const { return match_exact_; }
  public: void set_match_exact(bool match_exact) { match_exact_ = match_exact; }
  public: bool match_word() const { return match_word_; }
  public: void set_match_word(bool match_word) { match_word_ = match_word; }
  public: bool multiline() const { return multiline_; }
  public: void set_multiline(bool multiline) { multiline_ = multiline; }
  public: bool sticky() const { return sticky_; }
  public: void set_sticky(bool sticky) { sticky_ = sticky; }

  // dom::Dictionary
  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_regexp_init_h)
