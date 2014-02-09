// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode_factory.h"

#include "base/logging.h"
#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/config_mode.h"
#include "evita/text/modes/cxx_mode.h"
#include "evita/text/modes/haskell_mode.h"
#include "evita/text/modes/lisp_mode.h"
#include "evita/text/modes/mason_mode.h"
#include "evita/text/modes/perl_mode.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/text/modes/python_mode.h"
#include "evita/text/modes/xml_mode.h"

namespace text {

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

namespace {
class EnumProperty {
  private: uint m_nNth;
  private: Buffer::EnumChar m_oEnumChar;
  private: char16 m_wszName[100];
  private: char16 m_wszValue[100];

  public: EnumProperty(Buffer* pBuffer)
      : m_nNth(0),
        m_oEnumChar(pBuffer) {
    m_wszName[0] = 0;

    if (hasProperties())
      next();
  }

  public: bool AtEnd() const { return 0 == m_wszName[0]; }

  public: const char16* GetName() const {
    DCHECK(!AtEnd());
    return m_wszName;
  }

  public: const char16* GetValue() const {
    DCHECK(!AtEnd());
    return m_wszValue;
  }

  public: void Next() {
    DCHECK(!AtEnd());
    next();
  }

  private: void next() {
    enum State {
      State_Start,

      State_Name,
      State_Value,
      State_ValueStart,
    } eState = State_Start;

    auto nName = 0u;
    auto nValue = 0u;

    m_wszName[0] = 0;
    m_wszValue[0] = 0;

    while (!m_oEnumChar.AtEnd()) {
      auto const wch = m_oEnumChar.Get();
      m_oEnumChar.Next();

      switch (eState) {
        case State_Start:
          if (!IsWhitespace(wch)) {
            m_wszName[nName + 0] = wch;
            m_wszName[nName + 1] = 0;
            nName += 1;

            eState = State_Name;
          }
          break;

        case State_Name:
          if (':' == wch) {
            eState = State_ValueStart;
          } else if (nName < lengthof(m_wszName) - 1) {
            m_wszName[nName + 0] = wch;
            m_wszName[nName + 1] = 0;
            nName += 1;
          }
          break;

        case State_ValueStart:
          if (!IsWhitespace(wch)) {
            m_wszValue[nValue + 0] = wch;
            m_wszValue[nValue + 1] = 0;
            nValue += 1;
            eState = State_Value;
          }
          break;

        case State_Value:
          if (';' == wch) {
            m_nNth += 1;
             return;
          }

          if (nValue < lengthof(m_wszValue) - 1) {
            m_wszValue[nValue + 0] = wch;
            m_wszValue[nValue + 1] = 0;
            nValue += 1;
          }
          break;

        default:
          NOTREACHED();
      }
    }

    // for -*- lisp -*-
    if (!m_nNth && State_Name == eState) {
      ::lstrcpy(m_wszValue, m_wszName);
      ::lstrcpy(m_wszName, L"Mode");
    }
  }

  private: bool hasProperties() {
    enum State {
      State_Start,

      State_Dash, // -^
      State_DashStar, // -*^

      State_EndDash,
      State_EndDashStar,

      State_Properties, // after "-*-"
    } eState = State_Start;

    Posn lPropStart = 0;
    Posn lPropEnd = 0;

    while (!m_oEnumChar.AtEnd()) {
      auto const wch = m_oEnumChar.Get();
      m_oEnumChar.Next();

      if (Newline == wch)
        return false;

      switch (eState) {
        case State_Start:
          switch (wch) {
            case '-':
              eState = State_Dash;
              break;
          }
          break;

        case State_Dash:
          switch (wch) {
            case '*':
              eState = State_DashStar;
              break;
            case '-':
              break;
            default:
              eState = State_Start;
              break;
          }
          break;

      case State_DashStar:
          switch (wch) {
            case '-':
              eState = State_Properties;
              lPropStart = m_oEnumChar.GetPosn();
              break;
            default:
              eState = State_Start;
              break;
          }
          break;

      case State_Properties:
          switch (wch) {
            case '-':
              eState = State_EndDash;
              lPropEnd = m_oEnumChar.GetPosn() - 1;
              break;
          }
          break;

      case State_EndDash:
          switch (wch) {
            case '*':
              eState = State_EndDashStar;
              break;
            default:
              eState = State_Properties;
              break;
          }
          break;

      case State_EndDashStar:
          switch (wch) {
            case '-':
              m_oEnumChar.SetRange(lPropStart, lPropEnd);
              return true;

            default:
              eState = State_Properties;
              break;
          }
          break;

      default:
          NOTREACHED();
      }
    }
    return false;
  }
};

ModeFactoryes g_oModeFactoryes;
ModeFactory* s_pPlainTextModeFactory;

}   // namespace

ModeFactory* ModeFactory::Get(Buffer* pBuffer) {
  DCHECK(pBuffer);

  if (g_oModeFactoryes.IsEmpty()) {
      g_oModeFactoryes.Append(new ConfigModeFactory);
      g_oModeFactoryes.Append(new CxxModeFactory);
      g_oModeFactoryes.Append(new HaskellModeFactory);
      g_oModeFactoryes.Append(new LispModeFactory);
      g_oModeFactoryes.Append(new PerlModeFactory);
      g_oModeFactoryes.Append(new MasonModeFactory);
      g_oModeFactoryes.Append(new JavaModeFactory);
      s_pPlainTextModeFactory = new PlainTextModeFactory;
      g_oModeFactoryes.Append(s_pPlainTextModeFactory);
      g_oModeFactoryes.Append(new XmlModeFactory);
      g_oModeFactoryes.Append(new PythonModeFactory);
  }

  char16 wszMode[100];
  wszMode[0] = 0;
  foreach (EnumProperty, oEnum, pBuffer) {
    if (!::lstrcmpi(oEnum.GetName(), L"Mode")) {
      ::lstrcpy(wszMode, oEnum.GetValue());
      break;
    }
  }

  foreach (ModeFactoryes::Enum, oEnum, &g_oModeFactoryes) {
    auto const pModeFactory = oEnum.Get();
    if (!::lstrcmpi(pModeFactory->GetName(), wszMode))
      return pModeFactory;

    if (pModeFactory->IsSupported(pBuffer->GetFileName().c_str()))
      return pModeFactory;

    if (pModeFactory->IsSupported(pBuffer->name().c_str()))
      return pModeFactory;
  }

  return s_pPlainTextModeFactory;
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

  return State_Match == eState && 0 == *pwsz;
}

}  // namespace text
