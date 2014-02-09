// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_lexer_h)
#define INCLUDE_evita_dom_modes_lexer_h

#include "evita/text/buffer.h"

namespace text {

typedef HashTable_<StringKey, int, 100> KeywordTable;

//////////////////////////////////////////////////////////////////////
//
// LexerBase
//
class LexerBase {
  protected: Posn m_lTokenStart;
  protected: Buffer::ChangeTracker m_oChange;
  protected: Buffer::EnumChar m_oEnumChar;
  protected: Buffer* m_pBuffer;

  // ctor/dtor
  protected: LexerBase(Buffer*);
  protected: ~LexerBase();

  DISALLOW_COPY_AND_ASSIGN(LexerBase);
};

namespace NewLexer {

//////////////////////////////////////////////////////////////////////
//
// NewLexer::LexerBase
//
class LexerBase {
  private: class EnumChar : public Buffer::EnumChar {
    private: typedef Buffer::EnumChar Super;

    private: Count m_lCount;

    public: EnumChar(Buffer* buffer);
    public: ~EnumChar();

    public: bool AtLimit() const;
    public: base::char16 Get() const;
    public: void Next();
    public: void SetCounter(Count lCount) { m_lCount = lCount; }
  };

  protected: Buffer::ChangeTracker m_oChange;
  protected: EnumChar m_oEnumChar;
  protected: Buffer* m_pBuffer;
  protected: const uint* m_prgnCharSyntax;
  protected: KeywordTable* m_pKeywordTab;

  // ctor/dtor
  protected: LexerBase(Buffer*, KeywordTable*, const uint*);
  protected: ~LexerBase();

  // [A]
  protected: static void addKeywords(KeywordTable* keyword_table,
                                     const char16** keywords,
                                     size_t num_keywords);

  // [I]
  protected: static KeywordTable* installKeywords(const char16** keywords,
                                                  size_t num_keywords);
  protected: bool isConsChar(char16 wch) const;
  protected: bool isPunctChar(char16 wch) const;

  // [S]
  protected: void setColor(Posn lStart, Posn lEnd, uint32_t nSyntax,
                           uint32_t nColor);

  DISALLOW_COPY_AND_ASSIGN(LexerBase);
};

}  // namespace NewLexer
}  // namespace text

#endif //!defined(INCLUDE_evita_dom_modes_lexer_h)
