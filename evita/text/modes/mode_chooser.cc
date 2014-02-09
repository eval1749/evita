// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode_chooser.h"

#include "base/logging.h"
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

namespace {
//////////////////////////////////////////////////////////////////////
//
// EnumProperty
//
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
}  // namespace text

//////////////////////////////////////////////////////////////////////
//
// ModeChoser
//
ModeChooser::ModeChooser() {
  factories_.push_back(ConfigModeFactory::instance());
  factories_.push_back(CxxModeFactory::instance());
  factories_.push_back(HaskellModeFactory::instance());
  factories_.push_back(LispModeFactory::instance());
  factories_.push_back(new MasonModeFactory());
  factories_.push_back(new PerlModeFactory());
  factories_.push_back(JavaModeFactory::instance());
  plain_text_mode_factory_ = new PlainTextModeFactory();
  factories_.push_back(plain_text_mode_factory_);
  factories_.push_back(new PythonModeFactory());
  factories_.push_back(new XmlModeFactory());
}

ModeFactory* ModeChooser::Choose(Buffer* buffer) {
  char16 wszMode[100];
  wszMode[0] = 0;
  foreach (EnumProperty, oEnum, buffer) {
    if (!::lstrcmpi(oEnum.GetName(), L"Mode")) {
      ::lstrcpy(wszMode, oEnum.GetValue());
      break;
    }
  }

  for (auto factory : factories_) {
    if (!::lstrcmpi(factory->GetName(), wszMode))
      return factory;

    if (factory->IsSupported(buffer->GetFileName().c_str()))
      return factory;

    if (factory->IsSupported(buffer->name().c_str()))
      return factory;
  }

  return plain_text_mode_factory_;
}

}  // namespace text
