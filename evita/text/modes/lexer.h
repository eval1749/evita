// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_lexer_h)
#define INCLUDE_evita_dom_modes_lexer_h

#include <unordered_set>

#include "base/strings/string16.h"
#include "evita/text/buffer.h"
#include "evita/text/buffer_mutation_observer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ChangeTracker
//
class ChangeTracker :  public BufferMutationObserver {
  private: Buffer* buffer_;
  private: Posn m_lStart;
  private: Posn m_lEnd;

  public: ChangeTracker(Buffer* buffer);
  public: ~ChangeTracker();

  public: Posn GetStart() const { return m_lStart; }
  public: Posn GetEnd() const { return m_lEnd; }

  public: void Reset();

  // BufferMutationObserver
  private: virtual void DidDeleteAt(int offset, size_t length) override;
  private: virtual void DidInsertAt(int offset, size_t length) override;
  private: virtual void DidInsertBefore(int offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(ChangeTracker);
};

//////////////////////////////////////////////////////////////////////
//
// Lexer
//
class Lexer {
  protected: Lexer();
  public: virtual ~Lexer();

  public: virtual bool Run(Count hint) = 0;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

//////////////////////////////////////////////////////////////////////
//
// LexerBase
//
class LexerBase : public Lexer {
  protected: Posn m_lTokenStart;
  protected: ChangeTracker m_oChange;
  protected: Buffer::EnumChar m_oEnumChar;
  protected: Buffer* m_pBuffer;

  // ctor/dtor
  protected: LexerBase(Buffer*);
  protected: virtual ~LexerBase();

  DISALLOW_COPY_AND_ASSIGN(LexerBase);
};

namespace NewLexer {

//////////////////////////////////////////////////////////////////////
//
// KeywordTable
//
class KeywordTable {
  private: std::unordered_set<base::string16> keyword_set_;

  protected: KeywordTable();
  public: ~KeywordTable();

  public: void AddKeywords(const char16** keywrods, size_t num_keywords);
  public: bool IsKeyword(const base::string16& word) const;

  DISALLOW_COPY_AND_ASSIGN(KeywordTable);
};

//////////////////////////////////////////////////////////////////////
//
// NewLexer::LexerBase
//
class LexerBase : public Lexer {
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

  protected: ChangeTracker m_oChange;
  protected: EnumChar m_oEnumChar;
  protected: Buffer* m_pBuffer;
  protected: const uint* m_prgnCharSyntax;
  protected: KeywordTable* m_pKeywordTab;

  // ctor/dtor
  protected: LexerBase(Buffer*, KeywordTable*, const uint*);
  protected: virtual ~LexerBase();

  protected: bool isConsChar(char16 wch) const;
  protected: bool IsKeyword(const base::string16& word) const;
  protected: bool isPunctChar(char16 wch) const;

  // [S]
  protected: void setColor(Posn lStart, Posn lEnd, uint32_t nSyntax,
                           uint32_t nColor);

  DISALLOW_COPY_AND_ASSIGN(LexerBase);
};

}  // namespace NewLexer
}  // namespace text

#endif //!defined(INCLUDE_evita_dom_modes_lexer_h)
