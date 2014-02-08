// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/mode.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/dom/modes/config_mode.h"
#include "evita/dom/modes/cxx_mode.h"
#include "evita/dom/modes/haskell_mode.h"
#include "evita/dom/modes/lisp_mode.h"
#include "evita/dom/modes/mason_mode.h"
#include "evita/dom/modes/perl_mode.h"
#include "evita/dom/modes/plain_text_mode.h"
#include "evita/dom/modes/python_mode.h"
#include "evita/dom/modes/xml_mode.h"

#include <commctrl.h>
#include <string>
#include <unordered_map>

#pragma comment(lib, "comctl32.lib")

namespace text
{

namespace NewLexer
{

/// <summary>
///   Construct LexerBase.
/// </summary>
LexerBase::LexerBase(
    Buffer*         pBuffer,
    KeywordTable*   pKeywordTab,
    const uint*     prgnCharSyntax ) :
        m_oEnumChar(pBuffer),
        m_pBuffer(pBuffer),
        m_pKeywordTab(pKeywordTab),
        m_prgnCharSyntax(prgnCharSyntax)
{
    pBuffer->RegisterChangeTracker(&m_oChange);
} // LexerBase::LexerBase

/// <summary>
///   Destruct LexerBase.
/// </summary>
LexerBase::~LexerBase()
{
    m_pBuffer->UnregisterChangeTracker(&m_oChange);
} // LexerBase::~LexerBase

/// <summary>
///   Add keywords to keyword table.
/// </summary>
void
LexerBase::addKeywords(
    KeywordTable*   pKeyTab,
    const char16**  prgpwszKeyword,
    size_t          cKeywords )
{
    for (
        const char16** p = prgpwszKeyword;
        p < prgpwszKeyword + cKeywords;
        p++ )
    {
        pKeyTab->Put(new StringKey(*p), 1);
    } // for each keyword
} // LexerBase::addKeywords

/// <summary>
///   Install keyword table if needed.
/// </summary>
KeywordTable*
LexerBase::installKeywords(
    const char16**  prgpwszKeyword,
    size_t          cKeywords )
{
    KeywordTable* pKeyTab = new KeywordTable;
    addKeywords(pKeyTab, prgpwszKeyword, cKeywords);
    return pKeyTab;
} // LexerBase::installKeywords

} // NewLexer

//////////////////////////////////////////////////////////////////////
//
// LexerBase ctor
//
LexerBase::LexerBase(Buffer* pBuffer) :
    m_oEnumChar(pBuffer),
    m_pBuffer(pBuffer)
{
    pBuffer->RegisterChangeTracker(&m_oChange);
} // LexerBase::LexerBase


//////////////////////////////////////////////////////////////////////
//
// LexerBase ctor
//
LexerBase::~LexerBase()
{
    m_pBuffer->UnregisterChangeTracker(&m_oChange);
} // LexerBase::~LexerBase

/// <summary>
///   Construct Edit Mode object for pBuffer.
/// </summary>
Mode::Mode(ModeFactory* pClass, Buffer* pBuffer) :
    m_pBuffer(pBuffer),
    m_pClass(pClass),
    m_iIcon(-1)
{
    // nothing to do
} // Mode::Mode


/// <summary>
///   Denstruct Edit Mode object.
/// </summary>
Mode::~Mode()
{
    // nothing to do
} // Mode::~Mode

/// <summary>
///   Get syntax of specified character.
/// </summary>
//  Returns ANSIC C/POSIX(LC_TYPE)
//
// See WinNls.h for C1_xxx
//  C1_UPPER    0x001
//  C1_LOWER    0x002
//  C1_DIGIT    0x004
//  C1_SPACE    0x008
//  C1_PUNCT    0x010
//  C1_CNTRL     0x020
//  C1_BLANK    0x040
//  C1_XDIGIT   0x080
//  C1_ALPHA    0x100
//  C1_DEFINED  0x200
//
//    Code    Name      Type
//  +-------+---------+-------------------------------
//  | 0x09  | TAB     | C1_SPACE + C1_CNTRL + C1_BLANK
//  | 0x0A  | LF      | C1_SPACE + C1_CNTRL
//  | 0x0D  | CR      | C1_SPACE + C1_CNTRL
//  | 0x20  | SPACE   | C1_SPACE + C1_BLANK
//  +-------+---------+-------------------------------
//
uint ModeFactory::GetCharSyntax(char16 wch) const
{

    if (wch < 0x20)
    {
        return CharSyntax::Syntax_Control;
    }

    if (wch < 0x80)
    {
        uint nSyntax = m_prgnCharSyntax[wch - 0x20];
        if (0 != nSyntax)
        {
            return nSyntax;
        }
    }

    WORD wType;
    if (! ::GetStringTypeW(CT_CTYPE1, &wch, 1, &wType))
    {
        return CharSyntax::Syntax_None;
    }

    if (wType & (C1_ALPHA | C1_DIGIT))
    {
        return CharSyntax::Syntax_Word;
    }

    if (wType & (C1_BLANK | C1_SPACE))
    {
        return CharSyntax::Syntax_Whitespace;
    }

    if (wType & C1_PUNCT)
    {
        return CharSyntax::Syntax_Punctuation;
    }

    if (wType & C1_CNTRL)
    {
        return CharSyntax::Syntax_Control;
    }

    return CharSyntax::Syntax_None;
} // ModeFactory::GetCharSyntax

/// <summary>
///  Enumerate file property
/// </summary>
class EnumProperty
{
    private: uint               m_nNth;
    private: Buffer::EnumChar   m_oEnumChar;
    private: char16             m_wszName[100];
    private: char16             m_wszValue[100];

    public: EnumProperty(Buffer* pBuffer) :
        m_nNth(0),
        m_oEnumChar(pBuffer)
    {
        m_wszName[0]  = 0;

        if (hasProperties())
        {
            next();
        }
    } // EnumProperty

    public: bool AtEnd() const { return 0 == m_wszName[0]; }

    public: const char16* GetName() const
        { ASSERT(!AtEnd()); return m_wszName; }

    public: const char16* GetValue() const
        { ASSERT(!AtEnd()); return m_wszValue; }

    public: void Next()
        { ASSERT(!AtEnd()); next(); }

    private: void next()
    {
        enum State
        {
            State_Start,

            State_Name,
            State_Value,
            State_ValueStart,
        } eState = State_Start;

        uint nName  = 0;
        uint nValue = 0;

        m_wszName[0]  = 0;
        m_wszValue[0] = 0;

        while (! m_oEnumChar.AtEnd())
        {
            char16 wch = m_oEnumChar.Get();
            m_oEnumChar.Next();

            switch (eState)
            {
            case State_Start:
                if (! IsWhitespace(wch))
                {
                    m_wszName[nName + 0] = wch;
                    m_wszName[nName + 1] = 0;
                    nName += 1;

                    eState = State_Name;
                }
                break;

            case State_Name:
                if (':' == wch)
                {
                    eState = State_ValueStart;
                }
                else if (nName < lengthof(m_wszName) - 1)
                {
                    m_wszName[nName + 0] = wch;
                    m_wszName[nName + 1] = 0;
                    nName += 1;
                }
                break;

            case State_ValueStart:
                if (! IsWhitespace(wch))
                {
                    m_wszValue[nValue + 0] = wch;
                    m_wszValue[nValue + 1] = 0;
                    nValue += 1;

                    eState = State_Value;
                }
                break;

            case State_Value:
                if (';' == wch)
                {
                    m_nNth += 1;
                    return;
                }

                if (nValue < lengthof(m_wszValue) - 1)
                {
                    m_wszValue[nValue + 0] = wch;
                    m_wszValue[nValue + 1] = 0;
                    nValue += 1;
                }
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch eState
        } // for each char

        // for -*- lisp -*-
        if (0 == m_nNth && State_Name == eState)
        {
            ::lstrcpy(m_wszValue, m_wszName);
            ::lstrcpy(m_wszName, L"Mode");
        }
    } // next

    private: bool hasProperties()
    {
        enum State
        {
            State_Start,

            State_Dash,         // -^
            State_DashStar,     // -*^

            State_EndDash,
            State_EndDashStar,

            State_Properties,   // after "-*-"
        } eState = State_Start;

        Posn lPropStart = 0;
        Posn lPropEnd   = 0;

        while (! m_oEnumChar.AtEnd())
        {
            char16 wch = m_oEnumChar.Get();
            m_oEnumChar.Next();

            if (Newline == wch)
            {
                return false;
            }

            switch (eState)
            {
            case State_Start:
                switch (wch)
                {
                case '-':
                    eState = State_Dash;
                    break;
                } // swtich wch
                break;

            case State_Dash:
                switch (wch)
                {
                case '*':
                    eState = State_DashStar;
                    break;
                case '-':
                    break;
                default:
                    eState = State_Start;
                    break;
                } // swtich wch
                break;

            case State_DashStar:
                switch (wch)
                {
                case '-':
                    eState = State_Properties;
                    lPropStart = m_oEnumChar.GetPosn();
                    break;
                default:
                    eState = State_Start;
                    break;
                } // switch wch
                break;

            case State_Properties:
                switch (wch)
                {
                case '-':
                    eState = State_EndDash;
                    lPropEnd = m_oEnumChar.GetPosn() - 1;
                    break;
                } // switch wch
                break;

            case State_EndDash:
                switch (wch)
                {
                case '*':
                    eState = State_EndDashStar;
                    break;
                default:
                    eState = State_Properties;
                    break;
                } // switch wch
                break;

            case State_EndDashStar:
                switch (wch)
                {
                case '-':
                    m_oEnumChar.SetRange(lPropStart, lPropEnd);
                    return true;

                default:
                    eState = State_Properties;
                    break;
                } // switch wch
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch state
        } // for each char

        return false;
    } // hasProperties
}; // EnumProperty

ModeFactoryes g_oModeFactoryes;
static ModeFactory* s_pPlainTextModeFactory;

/// <summary>
///   Get Mode for specified buffer.
/// </summary>
ModeFactory* ModeFactory::Get(Buffer* pBuffer)
{
    DCHECK(pBuffer);

    if (g_oModeFactoryes.IsEmpty())
    {
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
    foreach (EnumProperty, oEnum, pBuffer)
    {
        if (0 == ::lstrcmpi(oEnum.GetName(), L"Mode"))
        {
            ::lstrcpy(wszMode, oEnum.GetValue());
            break;
        }
    } // for each mode

    foreach (ModeFactoryes::Enum, oEnum, &g_oModeFactoryes)
    {
        ModeFactory* pModeFactory = oEnum.Get();
        if (0 == ::lstrcmpi(pModeFactory->GetName(), wszMode))
        {
            return pModeFactory;
        }

        if (pModeFactory->IsSupported(pBuffer->GetFileName().c_str()))
        {
            return pModeFactory;
        }

        if (pModeFactory->IsSupported(pBuffer->name().c_str()))
        {
            return pModeFactory;
        }
    } // for each mode

    return s_pPlainTextModeFactory;
} // ModeFactory::Get

/// <summary>
///   Retruns true if editing mode supports.
/// </summary>
bool ModeFactory::IsSupported(const char16* pwszName) const
{
    ASSERT(NULL != pwszName);

    const char16* pwszExt = lstrrchrW(pwszName, '.');
    if (NULL == pwszExt)
    {
        // Name doesn't contain have no file extension.
        return false;
    }

    pwszExt++;  // skip dot

    enum State
    {
        State_Start,

        State_Match,
        State_Skip,
        State_SkipSpace,
    } eState = State_Start;

    const char16* pwsz = pwszExt;
    const char16* pwszRunner = getExtensions();
    while (0 != *pwszRunner)
    {
        char16 wch = *pwszRunner++;
        switch (eState)
        {
        case State_Start:
            if (0 == wch || IsWhitespace(wch))
            {
                return false;
            }

        firstChar:
            if (*pwszExt == wch)
            {
                pwsz = pwszExt + 1;
                eState = State_Match;
            }
            else
            {
                eState = State_Skip;
            }
            break;

        case State_Match:
            if (0 == wch)
            {
                return 0 == *pwsz;
            }

            if (IsWhitespace(wch))
            {
                if (0 == *pwsz)
                {
                    return true;
                }
                
                eState = State_SkipSpace;
                break;
            }

            if (*pwsz == wch)
            {
                pwsz++;
            }
            else
            {
                eState = State_Skip;
            }
            break;

        case State_Skip:
            if (0 == wch)
            {
                return false;
            }

            if (IsWhitespace(wch))
            {
                eState = State_SkipSpace;
            }
            break;

        case State_SkipSpace:
            if (0 == wch)
            {
                return false;
            }

            if (! IsWhitespace(wch))
            {
                pwsz = pwszExt;
                goto firstChar;
            }
            break;

        default:
            CAN_NOT_HAPPEN();
        } // switch state
    } // while

    return State_Match == eState && 0 == *pwsz;
} // ModeFactory::IsSupported

}  // namespace text
