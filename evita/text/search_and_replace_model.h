// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_text_search_and_replace_model_h)
#define INCLUDE_evita_text_search_and_replace_model_h

#include "base/strings/string16.h"

enum SearchFlag {
  SearchFlag_Backward = 1 << 0,
  SearchFlag_CasePreserve = 1 << 1,
  SearchFlag_IgnoreCase = 1 << 2,
  SearchFlag_InSelection = 1 << 3,
  SearchFlag_MatchWord = 1 << 4,
  SearchFlag_Regex = 1 << 5,
};

struct SearchParameters {
  base::string16 search_text_;
  int m_rgf;

  SearchParameters();
  ~SearchParameters();

  bool IsBackward() const { return m_rgf & SearchFlag_Backward; }
  bool IsCasePreserve() const { return m_rgf & SearchFlag_CasePreserve; }
  bool IsIgnoreCase() const { return m_rgf & SearchFlag_IgnoreCase; }
  bool IsInSelection() const { return m_rgf & SearchFlag_InSelection; }
  bool IsMatchWord() const { return m_rgf & SearchFlag_MatchWord; }
  bool IsRegex() const { return m_rgf & SearchFlag_Regex; }
};

namespace text {

enum Direction {
  kDirectionDown,
  kDirectionUp,
};

enum ReplaceIn {
  kReplaceInSelection,
  kReplaceInWhole,
};

enum ReplaceMode {
  kReplaceAll,
  kReplaceOne,
};

class SearchAndReplaceModel {
  private: Direction direction_;
  private: int flags_;
  private: ReplaceIn replace_in_;
  private: base::string16 replace_text_;
  private: base::string16 search_text_;

  public: SearchAndReplaceModel();
  public: ~SearchAndReplaceModel();

  public: Direction direction() const { return direction_; }
  public: void set_direction(Direction direction) { direction_ = direction; }

  public: bool is_backward() const { return flags_ & SearchFlag_Backward; }
  public: void set_is_backward(bool is_backward);
  public: bool is_case_preserve() const {
    return flags_ & SearchFlag_CasePreserve;
  }
  public: void set_is_case_preserve(bool is_case_preserver);
  public: bool is_ignore_case() const {
    return flags_ & SearchFlag_IgnoreCase;
  }
  public: void set_is_ignore_case(bool is_ignore_case);
  public: bool is_in_selection() const {
      return flags_ & SearchFlag_InSelection;
  }
  public: void set_is_in_selection(bool is_in_selection);
  public: bool is_match_word() const {
    return flags_ & SearchFlag_MatchWord;
  }
  public: void set_is_match_word(bool is_match_word);
  public: bool is_regex() const { return flags_ & SearchFlag_Regex; }
  public: void set_is_regex(bool is_regex);

  public: ReplaceIn replace_in() const { return replace_in_; }
  public: void set_replace_in(ReplaceIn replace_in) {
    replace_in_ = replace_in;
  }

  public: const base::string16& replace_text() const { return replace_text_; }
  public: void set_replace_text(const base::string16& text) {
    replace_text_ = text;
  }

  public: const base::string16& search_text() const { return search_text_; }
  public: void set_search_text(const base::string16& text) {
    search_text_ = text;
  }
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_search_and_replace_model_h)
