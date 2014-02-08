// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_mode_h)
#define INCLUDE_evita_dom_modes_mode_h

#include "base/logging.h"
#include "base/strings/string16.h"
#include "evita/text/buffer.h"
#include "evita/li_util.h"

namespace text {

typedef HashTable_<StringKey, int, 100> KeywordTable;

/// <summary>
/// Character syntax definition
/// </summary>
class CharSyntax {
  public: enum Syntax {
    Syntax_None = 0x0,

    Syntax_CloseParen = 0x1,
    Syntax_Control = 0x2,
    Syntax_Escape = 0x3,
    Syntax_LineComment = 0x4,
    Syntax_OpenParen = 0x5,
    Syntax_Punctuation = 0x6,
    Syntax_StringQuote = 0x7,
    Syntax_Whitespace = 0x8,
    Syntax_Word = 0x9,
  };

  enum Constant {
    Trait_PairShift = 4,
    Trait_SyntaxMask = (1 << Trait_PairShift) - 1,

    Trait_BlockCommentEnd1 = 0x0001 << 16,
    Trait_BlockCommentEnd2 = 0x0002 << 16,
    Trait_BlockCommentEnd = 0x0003 << 16,
    Trait_BlockCommentStart1 = 0x0010 << 16,
    Trait_BlockCommentStart2 = 0x0020 << 16,
    Trait_BlockCommentStart = 0x0030 << 16,
    Trait_LineCommentStart = 0x0100 << 16,
  };

  public: static char16 GetPairChar(uint nSyntax) {
    return static_cast<char16>((nSyntax >> Trait_PairShift) & 0xFF);
  }

  public: static Syntax GetSyntax(uint nSyntax) {
    return static_cast<Syntax>(nSyntax & Trait_SyntaxMask);
  }

  DISALLOW_COPY_AND_ASSIGN(CharSyntax);
};

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

}  // namespace text

class ModeFactory;

class ModeFactory : public DoubleLinkedNode_<ModeFactory> {
  protected: const uint32_t* m_prgnCharSyntax;

  public: ModeFactory(const uint* prgnCharSyntax);
  public: virtual ~ModeFactory();

  // [C]
  public: virtual Mode* Create(Buffer*) = 0;

  // [E]
  protected: virtual const char16* getExtensions() const { return L""; }

  // [G]
  public: virtual const char16* GetName() const = 0;
  public: static ModeFactory* Get(Buffer*);
  public: uint32_t GetCharSyntax(char16) const;

  // [I]
  public: virtual bool IsSupported(const char16*) const;
};

typedef DoubleLinkedList_<ModeFactory> ModeFactoryes;

class Mode {
  protected: Buffer* m_pBuffer;
  protected: ModeFactory* m_pClass;

  protected: Mode(ModeFactory*, Buffer*);
  public: virtual ~Mode();

  // [D]
  public: virtual bool DoColor(Count) = 0;

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }

  public: ModeFactory* GetClass() const { return m_pClass; }

  public: uint GetCharSyntax(char16 wch) const {
    return m_pClass->GetCharSyntax(wch);
  }

  public: virtual const char16* GetName() const {
    return m_pClass->GetName();
  }
};

} // namespace text

#endif //!defined(INCLUDE_evita_dom_modes_mode_h)
