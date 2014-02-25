// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/lexer.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/modes/char_syntax.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ChangeTracker
//
ChangeTracker::ChangeTracker(Buffer* buffer) : buffer_(buffer) {
  Reset();
  buffer_->AddObserver(this);
}

ChangeTracker::~ChangeTracker() {
  buffer_->RemoveObserver(this);
}

void ChangeTracker::Reset() {
  m_lStart = Posn_Max;
  m_lEnd = 0;
}

// BufferMutationObserver
void ChangeTracker::DidDeleteAt(Posn offset, size_t) {
  m_lStart = std::min(m_lStart, offset);
  m_lEnd = std::max(m_lEnd, buffer_->GetEnd());
}

void ChangeTracker::DidInsertAt(Posn offset, size_t text_length) {
  auto const change_end = static_cast<Posn>(offset + text_length);
  m_lStart = std::min(m_lStart, offset);
  m_lEnd = std::max(m_lEnd, change_end);
}

void ChangeTracker::DidInsertBefore(Posn offset, size_t text_length) {
  auto const change_end = static_cast<Posn>(offset + text_length);
  m_lStart = std::min(m_lStart, offset);
  m_lEnd = std::max(m_lEnd, change_end);
}

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
    : m_oChange(pBuffer),
      m_oEnumChar(pBuffer),
      m_pBuffer(pBuffer) {
}

LexerBase::~LexerBase() {
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
    : m_oChange(pBuffer),
      m_oEnumChar(pBuffer),
      m_pBuffer(pBuffer),
      m_pKeywordTab(pKeywordTab),
      m_prgnCharSyntax(prgnCharSyntax) {
}

LexerBase::~LexerBase() {
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

  StyleValues style_values;
  style_values.set_bgcolor(RGB(255, 255, 255));
  style_values.set_color(nColor);
  style_values.set_syntax(static_cast<int>(nSyntax));
  m_pBuffer->SetStyle(lStart, lEnd, style_values);
}

}  // namespace NewLexer
}  // namespace text
