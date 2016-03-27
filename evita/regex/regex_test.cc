// Copyright 2012-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <windows.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/regex/regex.h"
#include "gtest/gtest.h"

namespace Regex {

class Range {
 public:
  Range() : bound_(false), end_(0), start_(0) {}

  bool operator==(const Range& other) const {
    if (!bound_) {
      return !other.bound_;
    }
    return end_ == other.end_ && start_ == other.start_ && text_ == other.text_;
  }

  bool operator!=(const Range& other) const { return !operator==(other); }

  int end() const { return end_; }
  int start() const { return start_; }

  base::string16 text() const {
    return base::string16(text_.begin() + start_, text_.begin() + end_);
  }

  void Bind(const base::string16& text, int start, int end) {
    bound_ = true;
    end_ = end;
    start_ = start;
    text_ = text;
  }

  bool IsEmpty() const { return !bound_; }

  void Reset() {
    bound_ = false;
    end_ = start_ = 0;
    text_ = L"";
  }

 private:
  bool bound_;
  Posn end_;
  Posn start_;
  base::string16 text_;
};

class MatchContext final : public Regex::IMatchContext {
 public:
  MatchContext(IRegex* regex, int num_captures, const base::string16& source)
      : captures_(num_captures + 1),
        matched_(false),
        regex_(regex),
        source_(source) {}

  const std::vector<Range>& captures() const { return captures_; }
  bool matched() const { return matched_; }
  void set_matched(bool matched) { matched_ = matched; }

  bool BackwardFindCharCi(char16 pattern,
                          Posn* inout_posn,
                          Posn stop) const override {
    auto posn = *inout_posn;
    while (posn >= stop) {
      --posn;
      if (CharEqCi(GetChar(posn), pattern)) {
        *inout_posn = posn + 1;
        return true;
      }
    }
    return false;
  }

  bool BackwardFindCharCs(char16 pattern,
                          Posn* inout_posn,
                          Posn stop) const override {
    auto posn = *inout_posn;
    while (posn >= stop) {
      --posn;
      if (CharEqCs(GetChar(posn), pattern)) {
        *inout_posn = posn + 1;
        return true;
      }
    }
    return false;
  }

  bool CharEqCi(char16 a, char16 b) const {
    return CharDowncase(a) == CharDowncase(b);
  }

  bool CharEqCs(char16 a, char16 b) const { return a == b; }

  bool ForwardFindCharCi(char16 pattern,
                         Posn* inout_posn,
                         Posn stop) const override {
    auto posn = *inout_posn;
    while (posn < stop) {
      if (CharEqCi(GetChar(posn), pattern)) {
        *inout_posn = posn;
        return true;
      }
      ++posn;
    }
    return false;
  }

  bool ForwardFindCharCs(char16 pattern,
                         Posn* inout_posn,
                         Posn stop) const override {
    auto posn = *inout_posn;
    while (posn < stop) {
      if (CharEqCs(GetChar(posn), pattern)) {
        *inout_posn = posn;
        return true;
      }
      ++posn;
    }
    return false;
  }

  bool GetCapture(int index, Posn* out_start, Posn* out_end) const override {
    if (index >= captures_.size()) {
      return false;
    }
    if (captures_[index].IsEmpty()) {
      return false;
    }
    *out_start = captures_[index].start();
    *out_end = captures_[index].end();
    return true;
  }

  char16 GetChar(Posn posn) const override { return source_[posn]; }
  Posn GetEnd() const override { return static_cast<Posn>(source_.size()); }

  void GetInfo(Regex::SourceInfo* info) const override {
    info->m_lStart = GetStart();
    info->m_lEnd = GetEnd();
    info->m_lScanStart = info->m_lStart;
    info->m_lScanEnd = info->m_lEnd;
  }

  Posn GetStart() const override { return 0; }
  void ResetCapture(int index) override { captures_[index].Reset(); }

  void ResetCaptures() override {
    for (auto it = captures_.begin(); it != captures_.end(); ++it) {
      it->Reset();
    }
  }

  void SetCapture(int index, Posn start, Posn end) override {
    captures_[index].Bind(source_, start, end);
  }

  bool StringEqCi(const char16* pattern_start,
                  int size,
                  Posn posn) const override {
    auto const pattern_end = pattern_start + size;
    for (auto pattern = pattern_start; pattern < pattern_end; ++pattern) {
      if (posn >= GetEnd()) {
        return false;
      }
      if (!CharEqCi(*pattern, GetChar(posn))) {
        return false;
      }
      ++posn;
    }
    return true;
  }

  bool StringEqCs(const char16* pattern_start,
                  int size,
                  Posn posn) const override {
    auto const pattern_end = pattern_start + size;
    for (auto pattern = pattern_start; pattern < pattern_end; ++pattern) {
      if (posn >= GetEnd()) {
        return false;
      }
      if (!CharEqCi(*pattern, GetChar(posn))) {
        return false;
      }
      ++posn;
    }
    return true;
  }

 private:
  std::vector<Range> captures_;
  bool matched_;
  IRegex* regex_;
  const base::string16 source_;

  DISALLOW_COPY_AND_ASSIGN(MatchContext);
};

class Pattern final {
 public:
  Pattern(IRegex* regex, int num_captures)
      : error_code_(0),
        error_posn_(0),
        num_captures_(num_captures),
        regex_(regex) {}

  Pattern(int error_code, int error_posn)
      : error_code_(error_code),
        error_posn_(error_posn),
        num_captures_(0),
        regex_(nullptr) {}

  ~Pattern() {}

  int error_code() const { return error_code_; }
  int error_posn() const { return error_posn_; }

  static std::unique_ptr<Pattern> Compile(const base::string16& source,
                                          int flags) {
    Context context;
    auto const regex = Regex::Compile(&context, source.data(),
                                      static_cast<int>(source.size()), flags);
    if (regex)
      return std::make_unique<Pattern>(regex, context.num_captures());
    return std::make_unique<Pattern>(context.error_code(),
                                     context.error_posn());
  }

  std::unique_ptr<MatchContext> Match(const base::string16& source) {
    auto context =
        std::make_unique<MatchContext>(regex_, num_captures_, source);
    context->set_matched(StartMatch(regex_, context.get()));
    return std::move(context);
  }

 private:
  class Context final : public Regex::ICompileContext {
   public:
    Context() : error_code_(0), error_posn_(0), num_captures_(0) {}

    void* AllocRegex(size_t size, int num_captures) override {
      num_captures_ = num_captures;
      return new char[size];
    }

    int error_code() const { return error_code_; }
    int error_posn() const { return error_posn_; }
    int num_captures() const { return num_captures_; }
    bool SetCapture(int, const char16*) override { return true; }

    void SetError(int error_posn, int error_code) {
      error_code_ = error_code;
      error_posn_ = error_posn;
    }

   private:
    int error_code_;
    int error_posn_;
    int num_captures_;
  };

  int const error_code_;
  int const error_posn_;
  int const num_captures_;
  IRegex* const regex_;

  DISALLOW_COPY_AND_ASSIGN(Pattern);
};

class Result final {
 public:
  Result() : strings_(0) {}
  Result(base::StringPiece p1, base::StringPiece p2) : strings_(2) {
    strings_[0] = base::UTF8ToUTF16(p1);
    strings_[1] = base::UTF8ToUTF16(p2);
  }
  Result(base::StringPiece p1, base::StringPiece p2, base::StringPiece p3)
      : strings_(3) {
    strings_[0] = base::UTF8ToUTF16(p1);
    strings_[1] = base::UTF8ToUTF16(p2);
    strings_[2] = base::UTF8ToUTF16(p3);
  }
  Result(base::StringPiece p1,
         base::StringPiece p2,
         base::StringPiece p3,
         base::StringPiece p4)
      : strings_(4) {
    strings_[0] = base::UTF8ToUTF16(p1);
    strings_[1] = base::UTF8ToUTF16(p2);
    strings_[2] = base::UTF8ToUTF16(p3);
    strings_[3] = base::UTF8ToUTF16(p4);
  }
  explicit Result(const MatchContext& match)
      : strings_(match.captures().size()) {
    auto index = 0;
    for (auto it = match.captures().begin(); it != match.captures().end();
         ++it) {
      strings_[index] = it->text();
      ++index;
    }
  }
  explicit Result(base::StringPiece string) : strings_(1) {
    strings_[0] = base::UTF8ToUTF16(string);
  }

  bool operator==(const Result& other) const {
    return strings_ == other.strings_;
  }

  std::vector<base::string16>::const_iterator begin() const {
    return strings_.begin();
  }
  std::vector<base::string16>::const_iterator end() const {
    return strings_.end();
  }

 private:
  std::vector<base::string16> strings_;
};

::std::ostream& operator<<(::std::ostream& os, const Result& result) {
  os << "Result(";
  const char* comma = "";
  for (auto it = result.begin(); it != result.end(); ++it) {
    os << comma;
    comma = ", ";
    os << "\"" << *it << "\"";
  }
  os << ")";
  return os;
}

class RegexTest : public ::testing::Test {
 protected:
  Result Execute(base::StringPiece pattern_source8,
                 base::StringPiece source8,
                 int flags = 0) {
    base::string16 pattern_source = base::UTF8ToWide(pattern_source8);
    base::string16 source = base::UTF8ToWide(source8);
    std::unique_ptr<Pattern> pattern(Pattern::Compile(pattern_source, flags));
    if (pattern->error_code())
      return Result(base::StringPrintf("Regex compile failed at %d",
                                       pattern->error_posn()));

    std::unique_ptr<MatchContext> match(pattern->Match(source));
    return match->matched() ? Result(*match) : Result();
  }
};

TEST_F(RegexTest, Basic) {
  EXPECT_EQ(Result("foo"), Execute("foo", "foobar"));  // fixed-001

  EXPECT_EQ(Result("foo"), Execute("foo.*", "foo"));             // quant-001
  EXPECT_EQ(Result("foobar"), Execute("foo.*", "foobar"));       // quant-002
  EXPECT_EQ(Result("foobarb"), Execute("foo.*b", "foobarbaz"));  // quant-003
  EXPECT_EQ(Result("foobarb", "bar"),
            Execute("foo(.*)b", "foobarbaz"));  // capture-001
  EXPECT_EQ(Result("Regex compile failed at 1"),
            Execute("{foo}", "foo"));  // syntax-error-001
  EXPECT_EQ(Result("Regex compile failed at 1"),
            Execute("*foo*", "foo"));  // syntax-error-002
  EXPECT_EQ(Result("Regex compile failed at 1"),
            Execute("+foo+", "foo"));  // syntax-error-003
  EXPECT_EQ(Result("Regex compile failed at 1"),
            Execute("?foo?", "foo"));  // syntax-error-004
  EXPECT_EQ(Result("Regex compile failed at 5"),
            Execute("foo{}", "foo"));  // syntax-error-005
  EXPECT_EQ(Result("Regex compile failed at 5"),
            Execute("foo{bar}", "foo"));  // syntax-error-006
  EXPECT_EQ(Result("Regex compile failed at 5"),
            Execute("foo**", "foo"));  // syntax-error-007
  EXPECT_EQ(Result("Regex compile failed at 0"),
            Execute("(foo", "foo"));  // syntax-error-008
  EXPECT_EQ(Result("Regex compile failed at 4"),
            Execute("foo)", "foo"));  // syntax-error-009
  EXPECT_EQ(Result("Regex compile failed at 1"),
            Execute(")_", "foo"));
}

TEST_F(RegexTest, Smoke00) {
  EXPECT_EQ(Result(""), Execute("", ""));            // smoke/0001
  EXPECT_EQ(Result(""), Execute("", "a"));           // smoke/0002
  EXPECT_EQ(Result("a"), Execute("a", "a"));         // smoke/0003
  EXPECT_EQ(Result(), Execute("a", "bcd"));          // smoke/0004
  EXPECT_EQ(Result("a"), Execute("a", "--a"));       // smoke/0006
  EXPECT_EQ(Result(), Execute("a", ""));             // smoke/0007
  EXPECT_EQ(Result("ab"), Execute("ab", "ab"));      // smoke/0008
  EXPECT_EQ(Result("ab"), Execute("ab", "--ab--"));  // smoke/0009
  EXPECT_EQ(Result("ab"), Execute("ab", "--ab"));    // smoke/0010
  EXPECT_EQ(Result(), Execute("ab", ""));            // smoke/0011
  EXPECT_EQ(Result(), Execute("ab", "bcd"));         // smoke/0012
}

TEST_F(RegexTest, CharacterShorthands) {
  EXPECT_EQ(Result("\x07"), Execute("\a", "\x07"));    // smoke/0021
  EXPECT_EQ(Result("\x08"), Execute("[\b]", "\x08"));  // smoke/0022
  EXPECT_EQ(Result("\x1B"), Execute("\\e", "\x1B"));   // smoke/0023
  EXPECT_EQ(Result("\x0C"), Execute("\\f", "\x0C"));   // smoke/0024
  EXPECT_EQ(Result("\n"), Execute("\\n", "\n"));       // smoke/0025
  EXPECT_EQ(Result("\x0D"), Execute("\\r", "\x0D"));   // smoke/0026
  EXPECT_EQ(Result("\x09"), Execute("\\t", "\x09"));   // smoke/0027
  EXPECT_EQ(Result("\x0B"), Execute("\\v", "\x0B"));   // smoke/0028
  EXPECT_EQ(Result("="), Execute("\\075", "="));       // smoke/0029
  EXPECT_EQ(Result("a"), Execute("\\u0061", "a"));     // smoke/0030
  EXPECT_EQ(Result("\x18"), Execute("\\cX", "\x18"));  // smoke/0031
}

TEST_F(RegexTest, AnchorsAndZeroWidth) {
  EXPECT_EQ(Result(""), Execute("^", "abc"));    // smoke/0040
  EXPECT_EQ(Result("a"), Execute("^a", "abc"));  // smoke/0041
  EXPECT_EQ(Result(), Execute("^a", "bc"));      // smoke/0042
  EXPECT_EQ(Result("b"),
            Execute("^b", "a\nbc", Regex::Option_Multiline));  // smoke/0043
  EXPECT_EQ(Result(""),
            Execute("\\A", "a\nbc", Regex::Option_Multiline));  // smoke/0044
  EXPECT_EQ(Result("a"),
            Execute("\\Aa", "a\nbc", Regex::Option_Multiline));  // smoke/0045
  EXPECT_EQ(Result(),
            Execute("\\Ab", "a\nbc", Regex::Option_Multiline));  // smoke/0045
  EXPECT_EQ(Result(""), Execute("$", ""));                       // smoke/0046
  EXPECT_EQ(Result("abc"), Execute("abc$", "abc"));              // smoke/0047
  EXPECT_EQ(Result(), Execute("abc$", "abcd"));                  // smoke/0048
  EXPECT_EQ(Result(""),
            Execute("$", "", Regex::Option_Multiline));  // smoke/0049
  EXPECT_EQ(Result("abc"),
            Execute("abc$", "abc", Regex::Option_Multiline));  // smoke/0050
  EXPECT_EQ(Result("abc"),
            Execute("abc$", "abc\n", Regex::Option_Multiline));  // smoke/0051
  EXPECT_EQ(Result(),
            Execute("abc$", "abcd", Regex::Option_Multiline));  // smoke/0052
  EXPECT_EQ(Result("abc"),
            Execute("abc$", "abc\n", Regex::Option_Multiline));  // smoke/0053
  EXPECT_EQ(Result("abc"), Execute("abc$", "abc\ndef",
                                   Regex::Option_Multiline));  // smoke/0055
  EXPECT_EQ(Result("abc"),
            Execute("abc\\Z", "abc\n", Regex::Option_Multiline));  // smoke/0056
  EXPECT_EQ(Result(),
            Execute("abc\\Z", "abcd", Regex::Option_Multiline));  // smoke/0057
  EXPECT_EQ(Result("abc"),
            Execute("abc\\Z", "abc\n", Regex::Option_Multiline));  // smoke/0058
  EXPECT_EQ(Result(), Execute("abc\\Z", "abc\ndef",
                              Regex::Option_Multiline));    // smoke/0059
  EXPECT_EQ(Result(""), Execute("\\z", "abc"));             // smoke/0060
  EXPECT_EQ(Result("abc"), Execute("abc\\z", "abc"));       // smoke/0061
  EXPECT_EQ(Result(), Execute("abc\\z", "abcn"));           // smoke/0062
  EXPECT_EQ(Result(""), Execute("\\G", "abc"));             // smoke/0063
  EXPECT_EQ(Result(""), Execute("\\b", "abc"));             // smoke/0064
  EXPECT_EQ(Result("abc"), Execute("\\babc", "Aabc+abc"));  // smoke/0065
  EXPECT_EQ(Result(""), Execute("\\B", "abc "));            // smoke/0066
  EXPECT_EQ(Result("abc"), Execute("\\Babc", "Aabc+abc"));  // smoke/0067
  EXPECT_EQ(Result("abc"),
            Execute("(?#comment1)abc(?#comment2)", "abc"));  // smoke/0068
}

TEST_F(RegexTest, CharSet) {
  EXPECT_EQ(Result("1"), Execute("\\d", "1"));      // smoke/0070
  EXPECT_EQ(Result(), Execute("\\d", "x"));         // smoke/0071
  EXPECT_EQ(Result("1"), Execute("[\\d]", "1"));    // smoke/0072
  EXPECT_EQ(Result(), Execute("[\\d]", "x"));       // smoke/0073
  EXPECT_EQ(Result(), Execute("\\D", "1"));         // smoke/0080
  EXPECT_EQ(Result("x"), Execute("\\D", "x"));      // smoke/0081
  EXPECT_EQ(Result(), Execute("[\\D]", "1"));       // smoke/0082
  EXPECT_EQ(Result("x"), Execute("[\\D]", "x"));    // smoke/0083
  EXPECT_EQ(Result("a"), Execute("[abc]", "a"));    // smoke/0100
  EXPECT_EQ(Result(), Execute("[abc]", "x"));       // smoke/0101
  EXPECT_EQ(Result(), Execute("[^abc]", "a"));      // smoke/0102
  EXPECT_EQ(Result("x"), Execute("[^abc]", "x"));   // smoke/0103
  EXPECT_EQ(Result("a"), Execute("[a-f]", "a"));    // smoke/0104
  EXPECT_EQ(Result(), Execute("[a-f]", "x"));       // smoke/0105
  EXPECT_EQ(Result(), Execute("[^a-f]", "a"));      // smoke/0106
  EXPECT_EQ(Result("x"), Execute("[^a-f]", "x"));   // smoke/0107
  EXPECT_EQ(Result("-"), Execute("[-]", "-"));      // smoke/0110
  EXPECT_EQ(Result("a"), Execute("[-a]", "a"));     // smoke/0111
  EXPECT_EQ(Result("="), Execute("[--A]", "="));    // smoke/0112
  EXPECT_EQ(Result("-"), Execute("[*-\\-]", "-"));  // smoke/0114
  EXPECT_EQ(Result(),
            Execute("[\\a\\cH\\e\\f\\n\\r\\t\\v]", "-"));  // smoke/0115
}

TEST_F(RegexTest, Smoke200) {
  EXPECT_EQ(Result("abccfoo"), Execute("abc*foo", "abccfoo"));  // smoke/0200
  EXPECT_EQ(Result("abcfoo"), Execute("abc*foo", "abcfoo"));    // smoke/0201
  EXPECT_EQ(Result("abfoo"), Execute("abc*foo", "abfoo"));      // smoke/0202
  EXPECT_EQ(Result("abc"), Execute("[abc]+", "abcfoo"));        // smoke/0204
  EXPECT_EQ(Result("abccfoo"), Execute("abc+foo", "abccfoo"));  // smoke/0210
  EXPECT_EQ(Result("abcfoo"), Execute("abc+foo", "abcfoo"));    // smoke/0211
  EXPECT_EQ(Result(), Execute("abc+foo", "abfoo"));             // smoke/0212
  EXPECT_EQ(Result(), Execute("abc?foo", "abccfoo"));           // smoke/0220
  EXPECT_EQ(Result("abcfoo"), Execute("abc?foo", "abcfoo"));    // smoke/0221
  EXPECT_EQ(Result("abfoo"), Execute("abc?foo", "abfoo"));      // smoke/0222
  EXPECT_EQ(Result("abccfoo"),
            Execute("abc{1,3}foo", "abccfoo"));                   // smoke/0230
  EXPECT_EQ(Result("abcfoo"), Execute("abc{1,3}foo", "abcfoo"));  // smoke/0231
  EXPECT_EQ(Result(), Execute("abc{1,3}foo", "abfoox"));          // smoke/0232
  EXPECT_EQ(Result("abcccfoo"),
            Execute("abc{1,3}foo", "abcccfoo"));             // smoke/0233
  EXPECT_EQ(Result(), Execute("abc{1,3}foo", "abccccfoo"));  // smoke/0234
  EXPECT_EQ(Result("0123456789", ""),
            Execute("(\\d*)+", "0123456789"));                    // smoke/0235
  EXPECT_EQ(Result("", ""), Execute("(d*)+", "0123456789"));      // smoke/0236
  EXPECT_EQ(Result("aaaaaaa"), Execute("a{2,}", "aaaaaaa"));      // smoke/0237
  EXPECT_EQ(Result("abbxyz"), Execute("ab{2,}xyz", "abbxyz"));    // smoke/0238
  EXPECT_EQ(Result("abbxyz"), Execute("ab{2}xyz", "abbxyz"));     // smoke/0239
  EXPECT_EQ(Result("FoO"), Execute("(?i)foo", "FoO"));            // smoke/0240
  EXPECT_EQ(Result("FoObar"), Execute("(?i:foo)bar", "FoObar"));  // smoke/0241
  EXPECT_EQ(Result("$100", "100"),
            Execute("[$](?<foo>\\d+)", "$100"));  // smoke/0250
  EXPECT_EQ(Result("123123", "123"),
            Execute("(?<foo>\\d+)\\k<foo>", "123123"));        // smoke/0251
  EXPECT_EQ(Result("bar"), Execute("(?<=foo)bar", "foobar"));  // smoke/0260
  EXPECT_EQ(Result(), Execute("(?<=foo)bar", "fxxbar"));       // smoke/0261
}

TEST_F(RegexTest, Lookahead) {
  // Matches newline in TEXTAREA tag using lookbehing and lookahead
  // Note: second match is faield. Since, [^\\n<*] is faield at the first
  // newline.
  EXPECT_EQ(Result("\n"),
            Execute("(?<=<textarea[^>]*>[^\\n<]*)[\\n](?=[^<]*</textarea>)",
                    "<textarea>\nfoo\nbar\n</textarea>"));  // smoke/0262
  EXPECT_EQ(Result("bar"),
            Execute("(?(?<=foo)bar|baz)", "foobar"));              // smoke/0270
  EXPECT_EQ(Result(), Execute("(?(?<=foo)bar|baz)", "bar"));       // smoke/0271
  EXPECT_EQ(Result("baz"), Execute("(?(?<=foo)bar|baz)", "baz"));  // smoke/0272
  EXPECT_EQ(Result("foobar", "foo", ""),
            Execute("(?:(foo)|(FOO))(?(1)bar|baz)", "foobar"));  // smoke/0280
  EXPECT_EQ(Result("FOObaz", "", "FOO"),
            Execute("(?:(foo)|(FOO))(?(1)bar|baz)", "FOObaz"));    // smoke/0281
  EXPECT_EQ(Result("<small>"), Execute("<small.*?>", "<small>"));  // smoke/0300
  EXPECT_EQ(Result("<small color='red'>"),
            Execute("<small.*?>", "<small color='red'>"));  // smoke/0301
  EXPECT_EQ(Result("<small color='red'>", "color='red'"),
            Execute("<small\\s*(.*?)>", "<small color='red'>"));  // smoke/0302
  EXPECT_EQ(Result(), Execute("<small.+?>", "<small>"));          // smoke/0310
  EXPECT_EQ(Result("<small color='red'>"),
            Execute("<small.+?>", "<small color='red'>"));  // smoke/0311
  EXPECT_EQ(Result("<small color='red'>", "color='red'"),
            Execute("<small\\s*(.+?)>", "<small color='red'>"));  // smoke/0312
}

TEST_F(RegexTest, WordBoundary) {
  EXPECT_EQ(Result(), Execute("\\bfoo\\b", "foobar"));             // smoke/0320
  EXPECT_EQ(Result("foo"), Execute("\\bfoo\\b", "foo+bar"));       // smoke/0321
  EXPECT_EQ(Result("foo"), Execute("\\bfoo\\b", "this is foo."));  // smoke/0322
  EXPECT_EQ(Result(), Execute("\\bfoo\\b", "this is foobar."));    // smoke/0323
}

TEST_F(RegexTest, Alternative) {
  EXPECT_EQ(Result("foobaz", "foo"),
            Execute("(foo|bar)baz", "foobaz"));  // smoke/0400
  EXPECT_EQ(Result("barbaz", "bar"),
            Execute("(foo|bar)baz", "barbaz"));  // smoke/0401
  EXPECT_EQ(Result("foo", "foo", ""),
            Execute("(foo)|(bar)baz", "foobaz"));  // smoke/0402
  EXPECT_EQ(Result("barbaz", "", "bar"),
            Execute("(foo)|(bar)baz", "barbaz"));  // smoke/0403
}

TEST_F(RegexTest, BackwardSearch) {
  EXPECT_EQ(Result("foo"),
            Execute("foo", "foo", Regex::Option_Backward));  // smoke/1000
  EXPECT_EQ(Result("foo"),
            Execute("foo", "foo bar", Regex::Option_Backward));  // smoke/1001
  EXPECT_EQ(Result("foobaz"), Execute("foo\\S+", "foobar foobaz",
                                      Regex::Option_Backward));  // smoke/1002
  EXPECT_EQ(Result("foobaz", "baz"),
            Execute("foo(\\S+)", "foobar foobaz",
                    Regex::Option_Backward));  // smoke/1003
  EXPECT_EQ(Result("<small><b>foo</b></small>"),
            Execute("<small.*?>", "<small><b>foo</b></small>",
                    Regex::Option_Backward));  // smoke/1004
}

TEST_F(RegexTest, Capture) {
  // This parrent uses 177 element of control stack.
  EXPECT_EQ(Result("foo(1, 2, 123.4567890123456789012345)", "1", "2",
                   "123.4567890123456789012345"),
            Execute("foo\\((\\d+), (\\d+), ([\\d.]+)\\)",
                    "foo(1, 2, 123.4567890123456789012345)"));
}

}  // namespace Regex
