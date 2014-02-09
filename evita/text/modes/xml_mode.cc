// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/xml_mode.h"

#include "evita/text/modes/char_syntax.h"

namespace text
{


//////////////////////////////////////////////////////////////////////
//
// Map Token Type To color
//
static uint32
k_rgnToken2Color[XmlLexer::Style_Limit] =
{
    RGB(255,   0,  0),  // Style_Unknown
    RGB(  0,   0, 255), // Style_Keyword
    RGB(128,   0,   0), // Style_AttVal1
    RGB(128,   0,   0), // Style_AttVal2
    RGB(  0, 128,   0), // Style_Comment
    RGB(  0,   0, 128), // Style_EntityRef
    RGB(128,   0,   0), // Style_Tag
    RGB(  0,   0,   0), // Style_Text
    RGB(128,   0,   0), // Style_CData
    RGB(  0,   0, 128), // Style_Sym
    RGB(128,   0, 128), // Style_Att
}; // k_rgnToken2Color


//////////////////////////////////////////////////////////////////////
//
// Map State To Token Type
//
static XmlLexer::Style
k_rgnState2Token[XmlLexer::State_Limit] =
{
    XmlLexer::Style_Unknown,    // State_Error

    XmlLexer::Style_Text,       // State_Text
    XmlLexer::Style_Keyword,    // State_Lt
    XmlLexer::Style_Keyword,    // State_Amp

    XmlLexer::Style_Sym,        // State_Tag
    XmlLexer::Style_Att,        // State_Att
    XmlLexer::Style_AttVal1,    // State_AttVal1
    XmlLexer::Style_AttVal2,    // State_AttVal2

    XmlLexer::Style_Sym,        // State_Lt_Bang
    XmlLexer::Style_Sym,        // State_Lt_Bang_Dash

    XmlLexer::Style_Comment,    // State_Comment
    XmlLexer::Style_Comment,    // State_Comment_Dash
    XmlLexer::Style_Comment,    // State_Comment_Dash_Dash

    XmlLexer::Style_CData,      // State_Lt_Bang_LBrk
    XmlLexer::Style_CData,      // State_Lt_Bang_LBrk_C
    XmlLexer::Style_CData,      // State_Lt_Bang_LBrk_CD
    XmlLexer::Style_CData,      // State_Lt_Bang_LBrk_CDA
    XmlLexer::Style_CData,      // State_Lt_Bang_LBrk_CDAT
    XmlLexer::Style_Keyword,    // State_Lt_Bang_LBrk_CDATA
    XmlLexer::Style_CData,      // State_CData
    XmlLexer::Style_CData,      // State_CData_RBrk
    XmlLexer::Style_Keyword,    // State_CData_RBrk_RBrk
}; // k_rgnState2Token
// Cxx mode character syntax
static const uint
k_rgnXmlCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),  // 0x22 "
    CharSyntax::Syntax_Punctuation,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_Punctuation,                 // 0x27 '
    CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),  // 0x28 (
    CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),  // 0x29 )
    CharSyntax::Syntax_Punctuation,                 // 0x2A *
    CharSyntax::Syntax_Punctuation,                 // 0x2B +
    CharSyntax::Syntax_Punctuation,                 // 0x2C ,
    CharSyntax::Syntax_Punctuation,                 // 0x2D -
    CharSyntax::Syntax_Punctuation,                 // 0x2E .
    CharSyntax::Syntax_Punctuation,                 // 0x2F /

    CharSyntax::Syntax_Word,                        // 0x30 0
    CharSyntax::Syntax_Word,                        // 0x31 1
    CharSyntax::Syntax_Word,                        // 0x32 2
    CharSyntax::Syntax_Word,                        // 0x33 3
    CharSyntax::Syntax_Word,                        // 0x34 4
    CharSyntax::Syntax_Word,                        // 0x35 5
    CharSyntax::Syntax_Word,                        // 0x36 6
    CharSyntax::Syntax_Word,                        // 0x37 7
    CharSyntax::Syntax_Word,                        // 0x38 8
    CharSyntax::Syntax_Word,                        // 0x39 9
    CharSyntax::Syntax_Punctuation,                 // 0x3A :
    CharSyntax::Syntax_Punctuation,                 // 0x3B ;
    CharSyntax::Syntax_Punctuation,                 // 0x3C <
    CharSyntax::Syntax_Punctuation,                 // 0x3D =
    CharSyntax::Syntax_Punctuation,                 // 0x3E >
    CharSyntax::Syntax_Punctuation,                 // 0x3F ?

    CharSyntax::Syntax_Punctuation,                 // 0x40 @
    CharSyntax::Syntax_Word,                        // 0x41 A
    CharSyntax::Syntax_Word,                        // 0x42 B
    CharSyntax::Syntax_Word,                        // 0x43 C
    CharSyntax::Syntax_Word,                        // 0x44 D
    CharSyntax::Syntax_Word,                        // 0x45 E
    CharSyntax::Syntax_Word,                        // 0x46 F
    CharSyntax::Syntax_Word,                        // 0x47 G
    CharSyntax::Syntax_Word,                        // 0x48 H
    CharSyntax::Syntax_Word,                        // 0x49 I
    CharSyntax::Syntax_Word,                        // 0x4A J
    CharSyntax::Syntax_Word,                        // 0x4B K
    CharSyntax::Syntax_Word,                        // 0x4C L
    CharSyntax::Syntax_Word,                        // 0x4D M
    CharSyntax::Syntax_Word,                        // 0x4E N
    CharSyntax::Syntax_Word,                        // 0x4F O

    CharSyntax::Syntax_Word,                        // 0x50 P
    CharSyntax::Syntax_Word,                        // 0x51 Q
    CharSyntax::Syntax_Word,                        // 0x52 R
    CharSyntax::Syntax_Word,                        // 0x53 S
    CharSyntax::Syntax_Word,                        // 0x54 T
    CharSyntax::Syntax_Word,                        // 0x55 U
    CharSyntax::Syntax_Word,                        // 0x56 V
    CharSyntax::Syntax_Word,                        // 0x57 W
    CharSyntax::Syntax_Word,                        // 0x58 X
    CharSyntax::Syntax_Word,                        // 0x59 Y
    CharSyntax::Syntax_Word,                        // 0x5A Z
    CharSyntax::Syntax_OpenParen  | (0x5D << CharSyntax::Trait_PairShift),   // 0x5B [
    CharSyntax::Syntax_Punctuation,                 // 0x5C backslash(\)
    CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),   // 0x5D ]
    CharSyntax::Syntax_Punctuation,                 // 0x5E ^
    CharSyntax::Syntax_Punctuation,                 // 0x5F _

    CharSyntax::Syntax_Punctuation,                 // 0x60 `
    CharSyntax::Syntax_Word,                        // 0x61 a
    CharSyntax::Syntax_Word,                        // 0x62 b
    CharSyntax::Syntax_Word,                        // 0x63 c
    CharSyntax::Syntax_Word,                        // 0x64 d
    CharSyntax::Syntax_Word,                        // 0x65 e
    CharSyntax::Syntax_Word,                        // 0x66 f
    CharSyntax::Syntax_Word,                        // 0x67 g
    CharSyntax::Syntax_Word,                        // 0x68 h
    CharSyntax::Syntax_Word,                        // 0x69 i
    CharSyntax::Syntax_Word,                        // 0x6A j
    CharSyntax::Syntax_Word,                        // 0x6B k
    CharSyntax::Syntax_Word,                        // 0x6C l
    CharSyntax::Syntax_Word,                        // 0x6D m
    CharSyntax::Syntax_Word,                        // 0x6E n
    CharSyntax::Syntax_Word,                        // 0x6F o

    CharSyntax::Syntax_Word,                        // 0x70 p
    CharSyntax::Syntax_Word,                        // 0x71 q
    CharSyntax::Syntax_Word,                        // 0x72 r
    CharSyntax::Syntax_Word,                        // 0x73 s
    CharSyntax::Syntax_Word,                        // 0x74 t
    CharSyntax::Syntax_Word,                        // 0x75 u
    CharSyntax::Syntax_Word,                        // 0x76 v
    CharSyntax::Syntax_Word,                        // 0x77 w
    CharSyntax::Syntax_Word,                        // 0x78 x
    CharSyntax::Syntax_Word,                        // 0x79 y
    CharSyntax::Syntax_Word,                        // 0x7A z
    CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),   // 0x7B {
    CharSyntax::Syntax_Word,                        // 0x7C |
    CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),   // 0x7D }
    CharSyntax::Syntax_Word,                        // 0x7E ^
    CharSyntax::Syntax_Control,                     // 0x7F DEL
}; // k_rgnXmlCharSyntax

//////////////////////////////////////////////////////////////////////
//
// XmlLexer ctor
//
XmlLexer::XmlLexer(Buffer* pBuffer) :
    m_eState(State_Error),
    LexerBase(pBuffer)
{
} // XmlLexer::PerlLexter


//////////////////////////////////////////////////////////////////////
//
// XmlLexer::restart
//
void XmlLexer::restart()
{
    m_oEnumChar.SyncEnd();

    Posn lStart = 0;
    m_oChange.Reset();
    m_oEnumChar.GoTo(lStart);
    m_eState = State_Text;
    m_lTokenStart = lStart;
} // XmlLexer::restart


//////////////////////////////////////////////////////////////////////
//
// XmlLexer::processTag
//
void XmlLexer::processTag()
{
} // XmlLexer::processTag


//////////////////////////////////////////////////////////////////////
//
// XmlLexer::Run
//
bool XmlLexer::Run(Count lCount)
{
    Posn lChange = m_oChange.GetStart();
    if (m_oEnumChar.GetPosn() >= lChange)
    {
        // The buffer is changed since last scan.
        restart();
    }

    if (m_oEnumChar.AtEnd())
    {
        return false;
    }

    while (--lCount > 0)
    {
        if (m_oEnumChar.AtEnd())
        {
            if (State_Tag == m_eState)
            {
                processTag();
            }
            setColor(m_eState);
            return false;
        }

        char16 wch = m_oEnumChar.Get();
        runAux(wch);
        m_oEnumChar.Next();
    } // while

    return true;
} // XmlLexer::Run


//////////////////////////////////////////////////////////////////////
//
// XmlLexer::runAux
//
void XmlLexer::runAux(char16 wch)
{
    switch (m_eState)
    {
    case State_Text:
        switch (wch)
        {
        case '<':
            setStateStart(State_Lt);
            break;
        case '&':
            setStateStart(State_Amp);
            break;
        } // switch char
        break;

    case State_Amp:
        switch (wch)
        {
        case ';':
            setStateEnd(State_Text, 1);
            break;
        default:
            setStateCont(State_Amp);
            break;
        } // switch wch
        break;

    case State_Lt:
        switch (wch)
        {
        case '!':
            setStateCont(State_Lt_Bang);
            break;
        case '/':
        case '?':
            setStateCont(State_Tag);
            break;
        default:
            setStateCont(State_Tag);
            break;
        } // switch wch
        break;

    case State_Lt_Bang:
        switch (wch)
        {
        case '-':
            setStateCont(State_Lt_Bang_Dash);
            break;
        case '>':
            setStateEnd(State_Text);
            break;
        case OpenBracket:
            setStateCont(State_Lt_Bang_LBrk);
            break;
        default:
            setStateCont(State_Tag);
            break;
        } // switch wch
        break;

    case State_Lt_Bang_Dash:
        switch (wch)
        {
        case '-':
            setStateStart(State_Comment);
            break;
        default:
            setStateCont(State_Tag);
            break;
        } // switch wch
        break;

    #define case_State_(mp_first, mp_ch, mp_next, mp_cont) \
        case State_ ## mp_first :\
            switch (wch) \
            { \
            case mp_ch: \
                setStateCont(State_ ## mp_first ## mp_next); \
                break; \
            default: \
                setStateCont(State_ ## mp_cont); \
                break; \
            } \
            break;

    case_State_(Lt_Bang_LBrk,       'C', _C, Tag)
    case_State_(Lt_Bang_LBrk_C,     'D', D, Tag)
    case_State_(Lt_Bang_LBrk_CD,    'A', A, Tag)
    case_State_(Lt_Bang_LBrk_CDA,   'T', T, Tag)
    case_State_(Lt_Bang_LBrk_CDAT,  'A', A, Tag)

    case State_Lt_Bang_LBrk_CDATA:
        switch (wch)
        {
        case OpenBracket:
            m_lTokenStart = m_oEnumChar.GetPosn() - 8;
            setStateStart(State_CData, 1);
            break;
        default:
            setStateCont(State_Tag);
            break;
        } // switch wch
        break;

    case_State_(CData,      CloseBracket, _RBrk, CData);
    case_State_(CData_RBrk, CloseBracket, _RBrk, CData);

    case State_CData_RBrk_RBrk:
        switch (wch)
        {
        case '>':
            m_eState = State_CData;
            setStateEnd(State_Text, -3);
            m_eState = State_CData_RBrk_RBrk;
            setStateEnd(State_Text, 4);
            break;
        default:
            setStateCont(State_CData);
            break;
        } // switch wch
        break;

    case_State_(Comment,      '-', _Dash, Comment)
    case_State_(Comment_Dash, '-', _Dash, Comment)

    case State_Comment_Dash_Dash:
        switch (wch)
        {
        case '>':
            setStateEnd(State_Text, -3);
            break;
        default:
            setStateCont(State_Comment);
            break;
        } // switch wch
        break;

    case State_Tag:
        switch (wch)
        {
        case '>':
            setStateEnd(State_Text);
            break;
        default:
            if (IsWhitespace(wch))
            {
                setStateEnd(State_Att);
            }
            else
            {
                setStateCont(State_Tag);
            }
            break;
        } // swtich wch
        break;

    case State_Att:
        switch (wch)
        {
        case DoubleQuote:
            setStateStart(State_AttVal2);
            break;
        case SingleQuote:
            setStateStart(State_AttVal1);
            break;
        case '>':
            setStateEnd(State_Text);
            break;
        } // swtich wch
        break;

    case State_AttVal1:
        switch (wch)
        {
        case SingleQuote:
            setStateEnd(State_Att, 1);
            break;
        default:
            setStateCont(State_AttVal1);
        } // switch wch
        break;

    case State_AttVal2:
        switch (wch)
        {
        case DoubleQuote:
            setStateEnd(State_Att, 1);
            break;
        default:
            setStateCont(State_AttVal2);
        } // switch wch
        break;
    } // switch state
} // XmlLexer::runAux


void XmlLexer::setColor(State eState, int iExtra)
{
    Posn lTokenEnd = m_oEnumChar.GetPosn() + iExtra;

    if (m_lTokenStart != lTokenEnd)
    {
        StyleValues oStyleValues;
        oStyleValues.m_rgfMask =
            StyleValues::Mask_Background |
            StyleValues::Mask_Color;
        oStyleValues.m_crBackground = RGB(255, 255, 255);

        oStyleValues.m_crColor =
            k_rgnToken2Color[k_rgnState2Token[eState]];

        m_pBuffer->SetStyle(
            m_lTokenStart,
            lTokenEnd,
            &oStyleValues );

        m_lTokenStart = lTokenEnd;
    }
} // XmlLexer::setColor


void XmlLexer::setStateCont(State eState)
{
    if (m_eState != eState) setColor(m_eState);
    m_eState = eState;
} // XmlLexer::setStateCont


void XmlLexer::setStateEnd(State eState, int iExtra)
{
    setColor(m_eState, iExtra);
    m_eState = eState;
} // XmlLexer::setStateEnd


void XmlLexer::setStateStart(State eState, int iExtra)
{
    Posn lTokenEnd = m_oEnumChar.GetPosn() + iExtra;

    StyleValues oStyleValues;
    oStyleValues.m_rgfMask =
        StyleValues::Mask_Background |
        StyleValues::Mask_Color;
    oStyleValues.m_crBackground = RGB(255, 255, 255);

    if (m_lTokenStart != lTokenEnd)
    {
        oStyleValues.m_crColor =
                k_rgnToken2Color[k_rgnState2Token[m_eState]];

        m_pBuffer->SetStyle(
            m_lTokenStart,
            lTokenEnd,
            &oStyleValues );
    }

    m_lTokenStart = lTokenEnd;

    oStyleValues.m_crColor =
            k_rgnToken2Color[k_rgnState2Token[eState]];

    m_pBuffer->SetStyle(
        m_lTokenStart,
        m_lTokenStart + 1,
        &oStyleValues );

    m_eState = eState;
} // XmlLexer::setStateStart

//////////////////////////////////////////////////////////////////////
//
// XmlMode
//
XmlMode::XmlMode() {
}

XmlMode::~XmlMode() {
}

bool XmlMode::DoColor(Count lCount) {
  if (!lexer_)
    lexer_.reset(new XmlLexer(buffer()));
  return lexer_->Run(lCount);
}

const char16* XmlMode::GetName() const {
  return L"XML";
}

//////////////////////////////////////////////////////////////////////
//
// XmlModeFactory
//
XmlModeFactory::XmlModeFactory() {
}

XmlModeFactory::~XmlModeFactory() {
}

Mode* XmlModeFactory::Create() {
  return new XmlMode();
}

}  // namespace text
