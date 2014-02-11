// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_clike_lexer_h)
#define INCLUDE_evita_text_modes_clike_lexer_h

#include "evita/text/modes/lexer.h"

namespace text {

class ClikeLexer : public NewLexer::LexerBase {
  private: enum State {
    State_StartLine,

    State_Annotation,

    State_BlockComment,
    State_BlockComment_Star,

    State_DoubleQuote,
    State_DoubleQuote_Backslash,

    State_LineComment,
    State_LineComment_Backslash,

    State_Normal,

    State_SingleQuote,
    State_SingleQuote_Backslash,

    State_Sharp,
    State_Slash,

    State_Word,

    State_Max_1,
  };

  public: enum Syntax {
    Syntax_None,
    Syntax_Comment,
    Syntax_Operator,
    Syntax_String,
    Syntax_Word,
    Syntax_WordReserved,

    Syntax_Max_1,
  };

  private: struct Token {
    Syntax m_eSyntax;
    bool m_fPartial;
    Posn m_lEnd;
    Posn m_lStart;
    Posn m_lWordStart;
    char16 m_wchPrefix;

    Token() {
      Reset();
    }

    void Reset(Posn lPosn = 0) {
      m_eSyntax = Syntax_None;
      m_fPartial = false;
      m_lEnd = lPosn;
      m_lStart = lPosn;
      m_lWordStart = lPosn;
      m_wchPrefix = 0;
    }
  };

  private: State m_eState;
  private: Token m_oToken;
  private: char16 m_wchAnnotation;

  protected: ClikeLexer(Buffer* pBuffer, NewLexer::KeywordTable* pKeywordTab,
                        base::char16 wchAnnotation);
  public: ~ClikeLexer();

  private: void colorToken();
  private: Syntax endToken(base::char16 wch);
  private: Syntax getToken();
  private: base::string16 GetWord() const;
  private: void processToken();
  private: void restart();

  // Lexer
  private: virtual bool Run(Count lCount) override;

  DISALLOW_COPY_AND_ASSIGN(ClikeLexer);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_clike_lexer_h)
