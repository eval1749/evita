// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/python_mode.h"

#include "evita/dom/modes/char_syntax.h"
#include "evita/dom/modes/lexer.h"
#include "evita/dom/modes/mode.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Python Keywords
//
// FIXME 2007-07-22 yosi@msn.com Should we color Python special variables?
static const char16*
k_rgpwszPythonKeyword[] = {
  // Keywords
  L"False",
  L"None",
  L"True",
  L"and",
  L"as",
  L"assert",
  L"break",
  L"class",
  L"continue",
  L"def",
  L"del",
  L"elif",
  L"else",
  L"except",
  L"finally",
  L"for",
  L"from",
  L"global",
  L"if",
  L"import",
  L"in",
  L"is",
  L"lambda",
  L"nonlocal",
  L"not",
  L"or",
  L"pass",
  L"raise",
  L"return",
  L"try",
  L"while",
  L"with",
  L"yield",
};

// Cxx mode character syntax
static const uint
k_rgnPythonCharSyntax[0x80 - 0x20] = {
  CharSyntax::Syntax_Whitespace,          // 0x20
  CharSyntax::Syntax_Punctuation,         // 0x21 !
  CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),  // 0x22 "
  CharSyntax::Syntax_LineComment,         // 0x23 #
  CharSyntax::Syntax_Punctuation,         // 0x24 $
  CharSyntax::Syntax_Punctuation,         // 0x25 %
  CharSyntax::Syntax_Punctuation,         // 0x26 &
  CharSyntax::Syntax_StringQuote | (0x27 << CharSyntax::Trait_PairShift),  // 0x27 '
  CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),  // 0x28 (
  CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),  // 0x29)
  CharSyntax::Syntax_Punctuation,         // 0x2A *
  CharSyntax::Syntax_Punctuation,         // 0x2B +
  CharSyntax::Syntax_Punctuation,         // 0x2C ,
  CharSyntax::Syntax_Punctuation,         // 0x2D -
  CharSyntax::Syntax_Punctuation,         // 0x2E .
  CharSyntax::Syntax_Punctuation,         // 0x2F /

  CharSyntax::Syntax_Word,            // 0x30 0
  CharSyntax::Syntax_Word,            // 0x31 1
  CharSyntax::Syntax_Word,            // 0x32 2
  CharSyntax::Syntax_Word,            // 0x33 3
  CharSyntax::Syntax_Word,            // 0x34 4
  CharSyntax::Syntax_Word,            // 0x35 5
  CharSyntax::Syntax_Word,            // 0x36 6
  CharSyntax::Syntax_Word,            // 0x37 7
  CharSyntax::Syntax_Word,            // 0x38 8
  CharSyntax::Syntax_Word,            // 0x39 9
  CharSyntax::Syntax_Punctuation,         // 0x3A :
  CharSyntax::Syntax_Punctuation,         // 0x3B ;
  CharSyntax::Syntax_Punctuation,         // 0x3C <
  CharSyntax::Syntax_Punctuation,         // 0x3D =
  CharSyntax::Syntax_Punctuation,         // 0x3E >
  CharSyntax::Syntax_Punctuation,         // 0x3F ?

  CharSyntax::Syntax_Punctuation,         // 0x40 @
  CharSyntax::Syntax_Word,            // 0x41 A
  CharSyntax::Syntax_Word,            // 0x42 B
  CharSyntax::Syntax_Word,            // 0x43 C
  CharSyntax::Syntax_Word,            // 0x44 D
  CharSyntax::Syntax_Word,            // 0x45 E
  CharSyntax::Syntax_Word,            // 0x46 F
  CharSyntax::Syntax_Word,            // 0x47 G
  CharSyntax::Syntax_Word,            // 0x48 H
  CharSyntax::Syntax_Word,            // 0x49 I
  CharSyntax::Syntax_Word,            // 0x4A J
  CharSyntax::Syntax_Word,            // 0x4B K
  CharSyntax::Syntax_Word,            // 0x4C L
  CharSyntax::Syntax_Word,            // 0x4D M
  CharSyntax::Syntax_Word,            // 0x4E N
  CharSyntax::Syntax_Word,            // 0x4F O

  CharSyntax::Syntax_Word,            // 0x50 P
  CharSyntax::Syntax_Word,            // 0x51 Q
  CharSyntax::Syntax_Word,            // 0x52 R
  CharSyntax::Syntax_Word,            // 0x53 S
  CharSyntax::Syntax_Word,            // 0x54 T
  CharSyntax::Syntax_Word,            // 0x55 U
  CharSyntax::Syntax_Word,            // 0x56 V
  CharSyntax::Syntax_Word,            // 0x57 W
  CharSyntax::Syntax_Word,            // 0x58 X
  CharSyntax::Syntax_Word,            // 0x59 Y
  CharSyntax::Syntax_Word,            // 0x5A Z
  CharSyntax::Syntax_OpenParen  | (0x5D << CharSyntax::Trait_PairShift),   // 0x5B [
  CharSyntax::Syntax_Escape,            // 0x5C backslash(\)
  CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),   // 0x5D ]
  CharSyntax::Syntax_Word,            // 0x5E ^
  CharSyntax::Syntax_Word,            // 0x5F _

  CharSyntax::Syntax_Punctuation,         // 0x60 `
  CharSyntax::Syntax_Word,            // 0x61 a
  CharSyntax::Syntax_Word,            // 0x62 b
  CharSyntax::Syntax_Word,            // 0x63 c
  CharSyntax::Syntax_Word,            // 0x64 d
  CharSyntax::Syntax_Word,            // 0x65 e
  CharSyntax::Syntax_Word,            // 0x66 f
  CharSyntax::Syntax_Word,            // 0x67 g
  CharSyntax::Syntax_Word,            // 0x68 h
  CharSyntax::Syntax_Word,            // 0x69 i
  CharSyntax::Syntax_Word,            // 0x6A j
  CharSyntax::Syntax_Word,            // 0x6B k
  CharSyntax::Syntax_Word,            // 0x6C l
  CharSyntax::Syntax_Word,            // 0x6D m
  CharSyntax::Syntax_Word,            // 0x6E n
  CharSyntax::Syntax_Word,            // 0x6F o

  CharSyntax::Syntax_Word,            // 0x70 p
  CharSyntax::Syntax_Word,            // 0x71 q
  CharSyntax::Syntax_Word,            // 0x72 r
  CharSyntax::Syntax_Word,            // 0x73 s
  CharSyntax::Syntax_Word,            // 0x74 t
  CharSyntax::Syntax_Word,            // 0x75 u
  CharSyntax::Syntax_Word,            // 0x76 v
  CharSyntax::Syntax_Word,            // 0x77 w
  CharSyntax::Syntax_Word,            // 0x78 x
  CharSyntax::Syntax_Word,            // 0x79 y
  CharSyntax::Syntax_Word,            // 0x7A z
  CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),   // 0x7B {
  CharSyntax::Syntax_Word,            // 0x7C |
  CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),   // 0x7D }
  CharSyntax::Syntax_Word,            // 0x7E ^
  CharSyntax::Syntax_Control,           // 0x7F DEL
};

/// <summary>
///   A base class of C-like language.
/// </summary>
class PythonLexer : public NewLexer::LexerBase {
  public: enum State {
    State_Normal,

    State_DoubleQuote,
    State_DoubleQuote_DoubleQuote,
    State_DoubleQuote_LongString,
    State_DoubleQuote_LongString_BackSlash,
    State_DoubleQuote_LongString_DoubleQuote,
    State_DoubleQuote_LongString_DoubleQuote_DoubleQuote,
    State_DoubleQuote_ShortString,
    State_DoubleQuote_ShortString_BackSlash,

    State_LineComment,
    State_LineComment_BackSlash,

    State_Number,

    State_SingleQuote,
    State_SingleQuote_SingleQuote,
    State_SingleQuote_LongString,
    State_SingleQuote_LongString_BackSlash,
    State_SingleQuote_LongString_SingleQuote,
    State_SingleQuote_LongString_SingleQuote_SingleQuote,
    State_SingleQuote_ShortString,
    State_SingleQuote_ShortString_BackSlash,

    State_Word,

    State_Max_1,
  };

  public: enum Syntax {
    Syntax_None,
    Syntax_Comment,
    Syntax_Number,
    Syntax_Operator,
    Syntax_String,
    Syntax_Word,
    Syntax_WordReserved,

    Syntax_Max_1,
  };

  private: struct Token {
    Syntax  m_eSyntax;
    bool  m_fPartial;
    Posn  m_lEnd;
    Posn  m_lStart;

    Token() :
      m_eSyntax(Syntax_None),
      m_fPartial(false),
      m_lEnd(0),
      m_lStart(0) {}
  };

  private: static const uint32 k_rgnSyntax2Color[Syntax_Max_1];
  private: static KeywordTable* s_pKeywordTab;

  private: State      m_eState;
  private: Token      m_oToken;

  // ctor
  public: PythonLexer(Buffer* pBuffer)
    : NewLexer::LexerBase(
        pBuffer,
        initKeywords(),
        k_rgnPythonCharSyntax),
      m_eState(State_Normal) {}

  // [C]
  private: void colorToken() {
    setColor(
      m_oToken.m_lStart,
      m_oToken.m_lEnd,
      m_oToken.m_eSyntax,
      k_rgnSyntax2Color[m_oToken.m_eSyntax]);
  }

  // [E]
  private: Syntax endToken() {
    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
    m_eState = State_Normal;
    return m_oToken.m_eSyntax;
  }

  // [G]
  private: Syntax getToken() {
    if (m_oToken.m_fPartial) {
      m_oToken.m_fPartial = false;
    } else {
      m_oToken.m_eSyntax = Syntax_None;
      m_oToken.m_lStart  = m_oEnumChar.GetPosn();
      m_oToken.m_lEnd  = m_oEnumChar.GetPosn();
    }

    for (;;) {
      if (m_oEnumChar.AtLimit()) {
        // We reached at end of scanning area.
        m_oToken.m_fPartial = true;
        m_oToken.m_lEnd = m_oEnumChar.GetPosn() + 1;
        colorToken();
        return m_oToken.m_eSyntax;
      }

      auto const wch = m_oEnumChar.Get();
      switch (m_eState) {
        case State_DoubleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '"'
                ? State_DoubleQuote_DoubleQuote
                : wch == '\\'
                    ? State_DoubleQuote_ShortString_BackSlash
                    : State_DoubleQuote_ShortString;
          break;

        case State_DoubleQuote_DoubleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '"') {
            m_eState = State_DoubleQuote_LongString;
            break;
          }
          return endToken();

        case State_DoubleQuote_LongString:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '"'
                ? State_DoubleQuote_LongString_DoubleQuote
                : wch == '\\'
                    ? State_DoubleQuote_LongString_BackSlash
                    : State_DoubleQuote_LongString;
          break;

        case State_DoubleQuote_LongString_BackSlash:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = State_DoubleQuote_LongString;
          break;

        case State_DoubleQuote_LongString_DoubleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '"'
              ? State_DoubleQuote_LongString_DoubleQuote_DoubleQuote
              : State_DoubleQuote_LongString;
          break;

        case State_DoubleQuote_LongString_DoubleQuote_DoubleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '"') {
            m_oEnumChar.Next();
            return endToken();
          }
          m_eState = State_DoubleQuote_LongString;
          break;

        case State_DoubleQuote_ShortString:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '"') {
            m_oEnumChar.Next();
            return endToken();
          }

          if (wch == '\\')
            m_eState = State_DoubleQuote_ShortString_BackSlash;
          break;

        case State_DoubleQuote_ShortString_BackSlash:
          m_eState = State_DoubleQuote_ShortString;
          break;

        case State_LineComment:
          ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
          if (wch == 0x0A) {
            // We found end of line comment.
            return endToken();
          }

          if (wch == '\\')
            m_eState = State_LineComment_BackSlash;
          break;

        case State_LineComment_BackSlash:
          ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
          m_eState = State_LineComment;
          break;

        case State_Normal:
          if (0x09 == wch || ' ' == wch || 0x0A == wch)
            break;

          setColor(
              m_oToken.m_lStart,
              m_oEnumChar.GetPosn(),
              Syntax_None,
              k_rgnSyntax2Color[Syntax_None]);

          m_oToken.m_lStart = m_oEnumChar.GetPosn();

          switch (wch) {
            case '"':
              m_oToken.m_eSyntax = Syntax_String;
              m_eState = State_DoubleQuote;
              break;

            case '\'':
              m_oToken.m_eSyntax = Syntax_String;
              m_eState = State_SingleQuote;
              break;

            case '#':
              m_oToken.m_eSyntax = Syntax_Comment;
              m_eState = State_LineComment;
              break;

            default:
              if (wch >= '0' && wch <= '9') {
                m_oToken.m_eSyntax = Syntax_Number;
                m_eState = State_Number;
              } else if (isConsChar(wch)) {
                m_oToken.m_eSyntax = Syntax_Word;
                m_eState = State_Word;
              } else {
                m_oEnumChar.Next();
                m_oToken.m_eSyntax = Syntax_Operator;
                return endToken();
              }
              break;
            }
            break;

        case State_Number:
          if (!isConsChar(wch))
            return endToken();
          break;

        case State_SingleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '\''
                ? State_SingleQuote_SingleQuote
                : wch == '\\'
                    ? State_SingleQuote_ShortString_BackSlash
                    : State_SingleQuote_ShortString;
          break;

        case State_SingleQuote_SingleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '\'') {
            m_eState = State_SingleQuote_LongString;
            break;
          }
          return endToken();

        case State_SingleQuote_LongString:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '\''
                ? State_SingleQuote_LongString_SingleQuote
                : wch == '\\'
                    ? State_SingleQuote_LongString_BackSlash
                    : State_SingleQuote_LongString;
          break;

        case State_SingleQuote_LongString_BackSlash:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = State_SingleQuote_LongString;
          break;

        case State_SingleQuote_LongString_SingleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          m_eState = wch == '\''
              ? State_SingleQuote_LongString_SingleQuote_SingleQuote
              : State_SingleQuote_LongString;
          break;

        case State_SingleQuote_LongString_SingleQuote_SingleQuote:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '\'') {
            m_oEnumChar.Next();
            return endToken();
          }
          m_eState = State_SingleQuote_LongString;
          break;

        case State_SingleQuote_ShortString:
          ASSERT(m_oToken.m_eSyntax == Syntax_String);
          if (wch == '\'') {
            m_oEnumChar.Next();
            return endToken();
          }

          if (wch == '\\')
            m_eState = State_SingleQuote_ShortString_BackSlash;
          break;

        case State_SingleQuote_ShortString_BackSlash:
          m_eState = State_SingleQuote_ShortString;
          break;

        case State_Word:
          if (!isConsChar(wch))
            return endToken();
          break;

        default:
          CAN_NOT_HAPPEN();
      }
      m_oEnumChar.Next();
    }
  }

  // [I]
  private: static KeywordTable* initKeywords() {
    if (NULL == s_pKeywordTab) {
      s_pKeywordTab = installKeywords(
          k_rgpwszPythonKeyword,
          lengthof(k_rgpwszPythonKeyword));
    }
    return s_pKeywordTab;
  }

  // [P]
  private: void processToken() {
    if (m_oToken.m_fPartial)
      return;

    if (m_oToken.m_eSyntax != Syntax_Word)
      return;

    char16 wszWord[40];
    auto const cwchWord = m_oToken.m_lEnd - m_oToken.m_lStart;
    if (cwchWord >= lengthof(wszWord)) {
      // Word is too long
      return;
    }

    m_pBuffer->GetText(wszWord, m_oToken.m_lStart, m_oToken.m_lEnd);

    StringKey oWord(wszWord, cwchWord);
    if (!!m_pKeywordTab->Get(&oWord))
      m_oToken.m_eSyntax = Syntax_WordReserved;
  }

  // [R]
  /// <summary>
  ///   Find restart position.
  /// </summary>
  private: void restart() {
    m_oEnumChar.SyncEnd();

    DEBUG_PRINTF("Backtrack from %d\n", m_oChange.GetStart());

    m_eState = State_Normal;
    m_oToken.m_fPartial = false;

    Posn lStart = 0;
    foreach (
        Buffer::EnumCharRev,
        oEnum,
        Buffer::EnumCharRev::Arg(m_pBuffer, m_oChange.GetStart())) {
      auto const wch = oEnum.Get();
      if (0x20 == wch || 0x09 == wch || 0x0A == wch) {
        auto const pStyle = oEnum.GetStyle();
        if (Syntax_None == pStyle->GetSyntax()) {
          lStart = oEnum.GetPosn() - 1;
          break;
        }
      }
    }

    DEBUG_PRINTF("restart from %d state=%d\n", lStart, m_eState);
    m_oChange.Reset();
    m_oEnumChar.GoTo(lStart);
  }

  /// <summary>
  ///   Lexer entry point.
  /// </summary>
  public: bool Run(Count lCount) {
    auto const lChange = m_oChange.GetStart();
    if (m_oEnumChar.GetPosn() >= lChange) {
      // The buffer is changed since last scan.
      restart();
    }

    m_oEnumChar.SetCounter(lCount);

    while (!m_oEnumChar.AtLimit()) {
      getToken();
      processToken();
      colorToken();
    }

    return !m_oEnumChar.AtEnd();
  }

  DISALLOW_COPY_AND_ASSIGN(PythonLexer);
};

KeywordTable* PythonLexer::s_pKeywordTab;

const uint32
PythonLexer::k_rgnSyntax2Color[PythonLexer::Syntax_Max_1] = {
  RGB(  0,   0,   0), // Syntax_None
  RGB(  0, 128,   0), // Syntax_Comment
  RGB(  0,   0,   0), // Syntax_Number
  RGB(  0,   0,  51), // Syntax_Operator
  RGB(163,  21,  21), // Syntax_String
  RGB(  0,   0,   0), // Syntax_Word
  RGB(  0,   0, 255), // Syntax_WordReserved
};

/// <summary>
///   Python mode
/// </summary>
class PythonMode : public Mode {
  private: PythonLexer m_oLexer;

  // ctor/dtor
  public: PythonMode(ModeFactory* pFactory, Buffer* pBuffer)
    : m_oLexer(pBuffer),
      Mode(pFactory, pBuffer) {}

  // [D]
  public: virtual bool DoColor(Count lCount) override {
    return m_oLexer.Run(lCount);
  }

  DISALLOW_COPY_AND_ASSIGN(PythonMode);
};

/// <summary>
///  Construct PythonModeFactory object
/// </summary>
PythonModeFactory::PythonModeFactory()
  : ModeFactory(k_rgnPythonCharSyntax) {}

/// <summary>
///   Create a PythonModeFactory instance.
/// </summary>
Mode* PythonModeFactory::Create(Buffer* pBuffer) {
  return new PythonMode(this, pBuffer);
}

}  // namespace text
