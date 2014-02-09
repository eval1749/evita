// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode_factory.h"

#include "base/logging.h"
#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/mode_chooser.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ModeFactory
//
ModeFactory::ModeFactory(const uint* prgnCharSyntax)
    : m_prgnCharSyntax(prgnCharSyntax) {
}

ModeFactory::~ModeFactory() {
}

/// <summary>
/// Get syntax of specified character.
/// </summary>
// Returns ANSIC C/POSIX(LC_TYPE)
//
// See WinNls.h for C1_xxx
// C1_UPPER 0x001
// C1_LOWER 0x002
// C1_DIGIT 0x004
// C1_SPACE 0x008
// C1_PUNCT 0x010
// C1_CNTRL 0x020
// C1_BLANK 0x040
// C1_XDIGIT 0x080
// C1_ALPHA 0x100
// C1_DEFINED 0x200
//
// Code Name Type
// +-------+---------+-------------------------------
// | 0x09 | TAB | C1_SPACE + C1_CNTRL + C1_BLANK
// | 0x0A | LF | C1_SPACE + C1_CNTRL
// | 0x0D | CR | C1_SPACE + C1_CNTRL
// | 0x20 | SPACE | C1_SPACE + C1_BLANK
// +-------+---------+-------------------------------
//
uint32_t ModeFactory::GetCharSyntax(char16 wch) const {
  if (wch < 0x20)
    return CharSyntax::Syntax_Control;

  if (wch < 0x80) {
    uint32_t nSyntax = m_prgnCharSyntax[wch - 0x20];
    if (nSyntax)
      return nSyntax;
  }

  WORD wType;
  if (!::GetStringTypeW(CT_CTYPE1, &wch, 1, &wType))
    return CharSyntax::Syntax_None;

  if (wType & (C1_ALPHA | C1_DIGIT))
    return CharSyntax::Syntax_Word;

  if (wType & (C1_BLANK | C1_SPACE))
    return CharSyntax::Syntax_Whitespace;

  if (wType & C1_PUNCT)
    return CharSyntax::Syntax_Punctuation;

  if (wType & C1_CNTRL)
    return CharSyntax::Syntax_Control;

  return CharSyntax::Syntax_None;
}

ModeFactory* ModeFactory::Get(Buffer* buffer) {
  return ModeChooser::instance()->Choose(buffer);
}

bool ModeFactory::IsSupported(const char16* pwszName) const {
  DCHECK(pwszName);

  const char16* pwszExt = lstrrchrW(pwszName, '.');
  if (!pwszExt) {
    // Name doesn't contain have no file extension.
    return false;
  }

  pwszExt++; // skip dot

  enum State {
    State_Start,

    State_Match,
    State_Skip,
    State_SkipSpace,
  } eState = State_Start;

  auto pwsz = pwszExt;
  auto pwszRunner = getExtensions();
  while (*pwszRunner) {
    char16 wch = *pwszRunner++;
    switch (eState) {
      case State_Start:
        if (!wch || IsWhitespace(wch))
          return false;

      firstChar:
        if (*pwszExt == wch) {
          pwsz = pwszExt + 1;
          eState = State_Match;
        } else {
          eState = State_Skip;
        }
        break;

      case State_Match:
        if (!wch)
          return !*pwsz;

        if (IsWhitespace(wch)) {
          if (!*pwsz)
            return true;
          
          eState = State_SkipSpace;
          break;
        }

        if (*pwsz == wch)
          pwsz++;
        else
          eState = State_Skip;
        break;

      case State_Skip:
        if (!wch)
          return false;

        if (IsWhitespace(wch))
          eState = State_SkipSpace;
        break;

      case State_SkipSpace:
        if (!wch)
          return false;

        if (!IsWhitespace(wch)) {
          pwsz = pwszExt;
          goto firstChar;
        }
        break;

      default:
        NOTREACHED();
    }
  }

  return State_Match == eState && !*pwsz;
}

}  // namespace text
