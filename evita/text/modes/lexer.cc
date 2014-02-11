// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/lexer.h"

#include "base/logging.h"
#include "evita/text/modes/char_syntax.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Lexer
//
Lexer::Lexer() {
}

Lexer::~Lexer() {
}

//////////////////////////////////////////////////////////////////////
//
// LexerBase
//
LexerBase::LexerBase(Buffer* pBuffer)
    : m_oEnumChar(pBuffer),
      m_pBuffer(pBuffer) {
  pBuffer->RegisterChangeTracker(&m_oChange);
}

LexerBase::~LexerBase() {
  m_pBuffer->UnregisterChangeTracker(&m_oChange);
}

namespace NewLexer {

//////////////////////////////////////////////////////////////////////
//
// KeywordTable
//
KeywordTable::KeywordTable() {
}

KeywordTable::~KeywordTable() {
}

void KeywordTable::AddKeywords(const char16** prgpwszKeyword,
                               size_t cKeywords) {
  for (const char16** runner = prgpwszKeyword;
       runner < prgpwszKeyword + cKeywords; runner++) {
    keyword_set_.insert(*runner);
  }
}

bool KeywordTable::IsKeyword(const base::string16& word) const {
  return keyword_set_.find(word) != keyword_set_.end();
}

//////////////////////////////////////////////////////////////////////
//
// LexerBase::EnumChar
//
LexerBase::EnumChar::EnumChar(Buffer* buffer) : Super(buffer) {
}

LexerBase::EnumChar::~EnumChar() {
}

bool LexerBase::EnumChar::AtLimit() const {
  if (m_lCount <= 0)
      return true;
  return AtEnd();
}

base::char16 LexerBase::EnumChar::Get() const {
  DCHECK(!AtLimit());
  return Super::Get();
}

void LexerBase::EnumChar::Next() {
  DCHECK(!AtLimit());
  m_lCount -= 1;
  Super::Next();
}

//////////////////////////////////////////////////////////////////////
//
// LexerBase
//
LexerBase::LexerBase(Buffer* pBuffer, KeywordTable* pKeywordTab,
                     const uint* prgnCharSyntax)
    : m_oEnumChar(pBuffer),
      m_pBuffer(pBuffer),
      m_pKeywordTab(pKeywordTab),
      m_prgnCharSyntax(prgnCharSyntax) {
      pBuffer->RegisterChangeTracker(&m_oChange);
}

LexerBase::~LexerBase() {
  m_pBuffer->UnregisterChangeTracker(&m_oChange);
}

bool LexerBase::isConsChar(char16 wch) const {
  if (wch >= 0x7F)
    return true;
  if (wch <= 0x20)
    return false;
  uint32_t nSyntax = m_prgnCharSyntax[wch - 0x20];
  return CharSyntax::Syntax_Word == CharSyntax::GetSyntax(nSyntax);
}

bool LexerBase::IsKeyword(const base::string16& word) const {
  return m_pKeywordTab->IsKeyword(word);
}

bool LexerBase::isPunctChar(char16 wch) const {
  if (wch >= 0x7F)
    return true;
  if (wch <= 0x20)
    return false;
  uint32_t nSyntax = m_prgnCharSyntax[wch - 0x20];
  return CharSyntax::Syntax_Punctuation == CharSyntax::GetSyntax(nSyntax);
}

void LexerBase::setColor(Posn lStart, Posn lEnd, uint32_t nSyntax,
                         uint32_t nColor) {
  if (lStart >= lEnd)
    return;

  StyleValues oStyleValues;

  oStyleValues.m_rgfMask =
      StyleValues::Mask_Background |
      StyleValues::Mask_Color |
      StyleValues::Mask_Syntax;

  oStyleValues.m_crBackground = RGB(255, 255, 255);
  oStyleValues.m_crColor = nColor;
  oStyleValues.m_nSyntax = static_cast<int>(nSyntax);

  m_pBuffer->SetStyle(lStart, lEnd, &oStyleValues);
}

}  // namespace NewLexer
}  // namespace text
