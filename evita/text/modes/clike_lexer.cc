// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/clike_lexer.h"

#include "base/logging.h"
#include "evita/text/modes/char_syntax.h"

namespace text {

namespace {
// Cxx mode character syntax
static const uint32_t k_rgnCxxCharSyntax[0x80 - 0x20] = {
  CharSyntax::Syntax_Whitespace, // 0x20
  CharSyntax::Syntax_Punctuation, // 0x21 !
  CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift), // 0x22 "
  CharSyntax::Syntax_Punctuation, // 0x23 #
  CharSyntax::Syntax_Punctuation, // 0x24 $
  CharSyntax::Syntax_Punctuation, // 0x25 %
  CharSyntax::Syntax_Punctuation, // 0x26 &
  CharSyntax::Syntax_StringQuote | (0x27 << CharSyntax::Trait_PairShift), // 0x27 '
  CharSyntax::Syntax_OpenParen | (0x29 << CharSyntax::Trait_PairShift), // 0x28 (
  CharSyntax::Syntax_CloseParen | (0x28 << CharSyntax::Trait_PairShift), // 0x29)
  CharSyntax::Syntax_Punctuation, // 0x2A *
  CharSyntax::Syntax_Punctuation, // 0x2B +
  CharSyntax::Syntax_Punctuation, // 0x2C ,
  CharSyntax::Syntax_Punctuation, // 0x2D -
  CharSyntax::Syntax_Punctuation, // 0x2E .
  CharSyntax::Syntax_Punctuation, // 0x2F /

  CharSyntax::Syntax_Word, // 0x30 0
  CharSyntax::Syntax_Word, // 0x31 1
  CharSyntax::Syntax_Word, // 0x32 2
  CharSyntax::Syntax_Word, // 0x33 3
  CharSyntax::Syntax_Word, // 0x34 4
  CharSyntax::Syntax_Word, // 0x35 5
  CharSyntax::Syntax_Word, // 0x36 6
  CharSyntax::Syntax_Word, // 0x37 7
  CharSyntax::Syntax_Word, // 0x38 8
  CharSyntax::Syntax_Word, // 0x39 9
  CharSyntax::Syntax_Punctuation, // 0x3A :
  CharSyntax::Syntax_Punctuation, // 0x3B ;
  CharSyntax::Syntax_Punctuation, // 0x3C <
  CharSyntax::Syntax_Punctuation, // 0x3D =
  CharSyntax::Syntax_Punctuation, // 0x3E >
  CharSyntax::Syntax_Punctuation, // 0x3F ?

  CharSyntax::Syntax_Punctuation, // 0x40 @
  CharSyntax::Syntax_Word, // 0x41 A
  CharSyntax::Syntax_Word, // 0x42 B
  CharSyntax::Syntax_Word, // 0x43 C
  CharSyntax::Syntax_Word, // 0x44 D
  CharSyntax::Syntax_Word, // 0x45 E
  CharSyntax::Syntax_Word, // 0x46 F
  CharSyntax::Syntax_Word, // 0x47 G
  CharSyntax::Syntax_Word, // 0x48 H
  CharSyntax::Syntax_Word, // 0x49 I
  CharSyntax::Syntax_Word, // 0x4A J
  CharSyntax::Syntax_Word, // 0x4B K
  CharSyntax::Syntax_Word, // 0x4C L
  CharSyntax::Syntax_Word, // 0x4D M
  CharSyntax::Syntax_Word, // 0x4E N
  CharSyntax::Syntax_Word, // 0x4F O

  CharSyntax::Syntax_Word, // 0x50 P
  CharSyntax::Syntax_Word, // 0x51 Q
  CharSyntax::Syntax_Word, // 0x52 R
  CharSyntax::Syntax_Word, // 0x53 S
  CharSyntax::Syntax_Word, // 0x54 T
  CharSyntax::Syntax_Word, // 0x55 U
  CharSyntax::Syntax_Word, // 0x56 V
  CharSyntax::Syntax_Word, // 0x57 W
  CharSyntax::Syntax_Word, // 0x58 X
  CharSyntax::Syntax_Word, // 0x59 Y
  CharSyntax::Syntax_Word, // 0x5A Z
  CharSyntax::Syntax_OpenParen | (0x5D << CharSyntax::Trait_PairShift), // 0x5B [
  CharSyntax::Syntax_Escape, // 0x5C backslash(\)
  CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift), // 0x5D ]
  CharSyntax::Syntax_Punctuation, // 0x5E ^
  CharSyntax::Syntax_Word, // 0x5F _

  CharSyntax::Syntax_Punctuation, // 0x60 `
  CharSyntax::Syntax_Word, // 0x61 a
  CharSyntax::Syntax_Word, // 0x62 b
  CharSyntax::Syntax_Word, // 0x63 c
  CharSyntax::Syntax_Word, // 0x64 d
  CharSyntax::Syntax_Word, // 0x65 e
  CharSyntax::Syntax_Word, // 0x66 f
  CharSyntax::Syntax_Word, // 0x67 g
  CharSyntax::Syntax_Word, // 0x68 h
  CharSyntax::Syntax_Word, // 0x69 i
  CharSyntax::Syntax_Word, // 0x6A j
  CharSyntax::Syntax_Word, // 0x6B k
  CharSyntax::Syntax_Word, // 0x6C l
  CharSyntax::Syntax_Word, // 0x6D m
  CharSyntax::Syntax_Word, // 0x6E n
  CharSyntax::Syntax_Word, // 0x6F o

  CharSyntax::Syntax_Word, // 0x70 p
  CharSyntax::Syntax_Word, // 0x71 q
  CharSyntax::Syntax_Word, // 0x72 r
  CharSyntax::Syntax_Word, // 0x73 s
  CharSyntax::Syntax_Word, // 0x74 t
  CharSyntax::Syntax_Word, // 0x75 u
  CharSyntax::Syntax_Word, // 0x76 v
  CharSyntax::Syntax_Word, // 0x77 w
  CharSyntax::Syntax_Word, // 0x78 x
  CharSyntax::Syntax_Word, // 0x79 y
  CharSyntax::Syntax_Word, // 0x7A z
  CharSyntax::Syntax_OpenParen | (0x7D << CharSyntax::Trait_PairShift), // 0x7B {
  CharSyntax::Syntax_Word, // 0x7C |
  CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift), // 0x7D }
  CharSyntax::Syntax_Word, // 0x7E ^
  CharSyntax::Syntax_Control, // 0x7F DEL
};

const uint32_t k_rgnSyntax2Color[ClikeLexer::Syntax_Max_1] = {
  RGB( 0, 0, 0), // Syntax_None
  RGB( 0, 128, 0), // Syntax_Comment
  RGB( 0, 0, 33), // Syntax_Operator
  RGB(163, 21, 21), // Syntax_String
  RGB( 0, 0, 0), // Syntax_Word
  RGB( 0, 0, 255), // Syntax_WordReserved
};
}  // namespace

ClikeLexer::ClikeLexer(Buffer* pBuffer, NewLexer::KeywordTable* pKeywordTab,
                       char16 wchAnnotation)
    : NewLexer::LexerBase(pBuffer, pKeywordTab, k_rgnCxxCharSyntax),
      m_eState(State_StartLine),
      m_wchAnnotation(wchAnnotation) {
}

ClikeLexer::~ClikeLexer() {
}

void ClikeLexer::colorToken() {
  setColor(
      m_oToken.m_lStart,
      m_oToken.m_lEnd,
      m_oToken.m_eSyntax,
      k_rgnSyntax2Color[m_oToken.m_eSyntax]);
}

ClikeLexer::Syntax ClikeLexer::endToken(char16 wch) {
  m_oToken.m_lEnd = m_oEnumChar.GetPosn();
  m_eState = 0x0A == wch ? State_StartLine : State_Normal;
  return m_oToken.m_eSyntax;
}

ClikeLexer::Syntax ClikeLexer::getToken() {
  if (m_oToken.m_fPartial) {
    m_oToken.m_fPartial = false;
  } else {
    m_oToken.m_eSyntax = Syntax_None;
    m_oToken.m_lStart = m_oEnumChar.GetPosn();
    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
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

  tryAgain:
    switch (m_eState) {
      case State_Annotation:
        if (isConsChar(wch)) {
          m_oToken.m_eSyntax = Syntax_Word;
          m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
          m_oToken.m_wchPrefix = m_wchAnnotation;
          m_eState = State_Word;
        } else {
          m_eState = State_Normal;
          goto tryAgain;
        }
        break;

      case State_BlockComment:
        DCHECK_EQ(Syntax_Comment, m_oToken.m_eSyntax);
        switch (wch) {
          case '*':
            m_eState = State_BlockComment_Star;
            break;
        }
        break;

      case State_BlockComment_Star:
        DCHECK_EQ(Syntax_Comment, m_oToken.m_eSyntax);
        switch (wch) {
          case '*':
            break;

          case '/':
            m_oEnumChar.Next();
            return endToken(wch);

          default:
            m_eState = State_BlockComment;
            break;
        }
        break;

      case State_DoubleQuote:
        DCHECK_EQ(Syntax_String, m_oToken.m_eSyntax);
        switch (wch) {
          case '"':
            m_oEnumChar.Next();
            return endToken(wch);

          case '\\':
            m_eState = State_DoubleQuote_Backslash;
            break;
        }
        break;

      case State_DoubleQuote_Backslash:
        DCHECK_EQ(Syntax_String, m_oToken.m_eSyntax);
        m_eState = State_DoubleQuote;
        break;

      case State_LineComment:
        DCHECK_EQ(Syntax_Comment, m_oToken.m_eSyntax);
        switch (wch) {
          case 0x0A:
            // We found end of line comment.
            return endToken(wch);

          case '\\':
            m_eState = State_LineComment_Backslash;
            break;
        }
        break;

      case State_LineComment_Backslash:
        DCHECK_EQ(Syntax_Comment, m_oToken.m_eSyntax);
        m_eState = State_LineComment;
        break;

      case State_Normal:
        if (0x0A == wch) {
          m_eState = State_StartLine;
          break;
        }

        if (0x09 == wch || ' ' == wch)
          break;

        setColor(m_oToken.m_lStart, m_oEnumChar.GetPosn(), Syntax_None,
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

          case '/':
            m_oToken.m_eSyntax = Syntax_Operator;
            m_eState = State_Slash;
            break;

          default:
            if (m_wchAnnotation == wch) {
              m_eState = State_Annotation;
            } else if (isConsChar(wch)) {
              m_oToken.m_eSyntax = Syntax_Word;
              m_oToken.m_lWordStart = m_oToken.m_lStart;
              m_oToken.m_wchPrefix = 0;
              m_eState = State_Word;
            } else {
              m_oEnumChar.Next();
              m_oToken.m_eSyntax = Syntax_Operator;
              return endToken(wch);
            }
            break;
        }
        break;

      case State_SingleQuote:
        DCHECK_EQ(Syntax_String, m_oToken.m_eSyntax);
        switch (wch) {
          case '\'':
            m_oEnumChar.Next();
            return endToken(wch);

          case '\\':
            m_eState = State_SingleQuote_Backslash;
            break;
        }
        break;

      case State_SingleQuote_Backslash:
        DCHECK_EQ(Syntax_String, m_oToken.m_eSyntax);
        m_eState = State_SingleQuote;
        break;

      case State_Sharp:
        switch (wch) {
          case 0x0A:
            m_eState = State_StartLine;
            break;

          case 0x09:
          case 0x20:
            break;

          default:
            if (isConsChar(wch)) {
              m_oToken.m_wchPrefix = '#';
              m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
              m_eState = State_Word;
            } else {
              m_eState = State_Normal;
            }
            break;
        }
        break;

      case State_Slash:
        switch (wch) {
          case '/':
            m_oToken.m_eSyntax = Syntax_Comment;
            m_eState = State_LineComment;
            break;

          case '*':
            m_oToken.m_eSyntax = Syntax_Comment;
            m_eState = State_BlockComment;
            break;

          default:
            return endToken(wch);
        }
        break;

      case State_StartLine:
        switch (wch) {
          case 0x09:
          case 0x0A:
          case 0x20:
            break;

          case '#':
            m_oToken.m_eSyntax = Syntax_Word;
            m_eState = State_Sharp;
            break;

          default:
            m_eState = State_Normal;
            goto tryAgain;
        }
        break;

      case State_Word:
        DCHECK_EQ(Syntax_Word, m_oToken.m_eSyntax);
        if (!isConsChar(wch))
          return endToken(wch);
        break;

    default:
        NOTREACHED();
    }

    m_oEnumChar.Next();
  }
}

base::string16 ClikeLexer::GetWord() const {
  auto const kMaxWordLength = 40;

  if (auto const prefix = m_oToken.m_wchPrefix) {
    auto const word_length = m_oToken.m_lEnd - m_oToken.m_lWordStart + 1;
    if (word_length > kMaxWordLength)
      return base::string16();
    base::string16 word(static_cast<size_t>(word_length), '?');
    word[0] = prefix;
    m_pBuffer->GetText(&word[1], m_oToken.m_lWordStart, m_oToken.m_lEnd);
    return word;
  }
  auto const word_length = m_oToken.m_lEnd - m_oToken.m_lWordStart;
  return word_length > kMaxWordLength ? base::string16() :
      m_pBuffer->GetText(m_oToken.m_lWordStart, m_oToken.m_lEnd);
}

void ClikeLexer::processToken() {
  if (m_oToken.m_fPartial)
    return;

  switch (m_oToken.m_eSyntax) {
    case Syntax_Word:
      break;

    default:
      return;
  }

  if (IsKeyword(GetWord()))
    m_oToken.m_eSyntax = Syntax_WordReserved;
}

void ClikeLexer::restart() {
  m_oEnumChar.SyncEnd();

  #if DEBUG_LEXER
    DEBUG_PRINTF("Backtrack from %d\n", m_oChange.GetStart());
  #endif

  m_eState = State_StartLine;

  Posn lStart = 0;
  foreach (Buffer::EnumCharRev, oEnum,
           Buffer::EnumCharRev::Arg(m_pBuffer, m_oChange.GetStart())) {
    auto const wch = oEnum.Get();
    if (0x20 == wch || 0x09 == wch || 0x0A == wch) {
      const auto& style = oEnum.GetStyle();
      if (Syntax_None == style.syntax()) {
        lStart = oEnum.GetPosn() - 1;
        if (0x0A != wch)
          m_eState = State_Normal;
        break;
      }
    }
  }

  #if DEBUG_LEXER
    DEBUG_PRINTF("restart from %d state=%d\n", lStart, m_eState);
  #endif

  m_oToken.Reset(lStart);
  m_oChange.Reset();
  m_oEnumChar.GoTo(lStart);
}

bool ClikeLexer::Run(Count lCount) {
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

}  // namespace text
