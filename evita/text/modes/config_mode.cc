// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/config_mode.h"

#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/lexer.h"
#include "evita/text/modes/mode.h"

namespace text
{

// Config mode character syntax
static const uint
k_rgnConfigCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),   // 0x22 "
    CharSyntax::Syntax_LineComment,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_Punctuation,                 // 0x27 '
    CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),   // 0x28 (
    CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),   // 0x29 )
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
    CharSyntax::Syntax_OpenParen  | (0x5D << CharSyntax::Trait_PairShift),    // 0x5B [
    CharSyntax::Syntax_Escape,                      // 0x5C backslash(\)
    CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),    // 0x5D ]
    CharSyntax::Syntax_Word,                        // 0x5E ^
    CharSyntax::Syntax_Word,                        // 0x5F _

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
    CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),    // 0x7B {
    CharSyntax::Syntax_Word,                        // 0x7C |
    CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),    // 0x7D }
    CharSyntax::Syntax_Word,                        // 0x7E ^
    CharSyntax::Syntax_Control,                     // 0x7F DEL
}; // k_rgnConfigCharSyntax

/// <summary>
///  Lexer for Config file
/// </summary>
class ConfigLexer : public NewLexer::LexerBase
{
    private: enum State
    {
        State_Comment,
        State_Normal,
        State_String,
        State_StringEscape,
        State_Word,
    }; // State

    private: enum Syntax
    {
        Syntax_None,
        Syntax_Comment,
        Syntax_String,
        Syntax_Word,

        Syntax_Limit,
    }; // Sytnax

    private: struct Token
    {
        Syntax  m_eSyntax;
        Posn    m_lEnd;
        Posn    m_lStart;
    }; // Token

    private: static const uint32 k_rgnSyntax2Color[Syntax_Limit];

    private: State m_eState;

    // ctor
    public: ConfigLexer(Buffer* pBuffer) :
        NewLexer::LexerBase(pBuffer, NULL, k_rgnConfigCharSyntax),
        m_eState(State_Normal) {}

    private: Token getToken()
    {
        Token oToken;
        oToken.m_eSyntax = Syntax_None;
        oToken.m_lStart  = m_oEnumChar.GetPosn();
        oToken.m_lEnd    = oToken.m_lStart;

        for (;;)
        {
            if (m_oEnumChar.AtLimit())
            {
                // We reached at end of scanning area.
                oToken.m_lEnd = m_oEnumChar.GetPosn();
                return oToken;
            }

            char16 wch = m_oEnumChar.Get();
            switch (m_eState)
            {
            case State_Comment:
                switch (wch)
                {
                case 0x0A:
                    // We found end of line comment.
                    setColor(
                        oToken.m_lStart,
                        m_oEnumChar.GetPosn() + 1,
                        Syntax_Comment,
                        k_rgnSyntax2Color[Syntax_Comment] );

                    m_eState = State_Normal;

                    oToken.m_lStart = m_oEnumChar.GetPosn() + 1;
                    oToken.m_lEnd   = oToken.m_lStart;
                    break;
                } // switch wch
                break;

            case State_Normal:
                if (0x09 == wch || 0x0A == wch || ' ' == wch)
                {
                    break;
                }

                setColor(
                    oToken.m_lStart,
                    m_oEnumChar.GetPosn(),
                    Syntax_None,
                    k_rgnSyntax2Color[Syntax_None] );

                oToken.m_lStart  = m_oEnumChar.GetPosn();

                switch (wch)
                {
                case '"':
                    oToken.m_eSyntax = Syntax_String;
                    m_eState = State_String;
                    break;

                case '#':
                    oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_Comment;
                    break;

                default:
                    oToken.m_eSyntax = Syntax_Word;
                    m_eState = State_Word;
                    break;
                } // switch wch
                break;

            case State_String:
                switch (wch)
                {
                case '"':
                    oToken.m_eSyntax = Syntax_String;
                    oToken.m_lEnd    = m_oEnumChar.GetPosn() + 1;
                    m_oEnumChar.Next();
                    m_eState = State_Normal;
                    return oToken;

                case '\\':
                    m_eState = State_StringEscape;
                    break;
                } // switch wch
                break;

            case State_StringEscape:
                m_eState = State_String;
                break;

            case State_Word:
                switch (wch)
                {
                case 0x09:
                case 0x0A:
                case ' ':
                case '"':
                case '#':
                    oToken.m_eSyntax = Syntax_Word;
                    oToken.m_lEnd = m_oEnumChar.GetPosn();
                    m_eState = State_Normal;
                    return oToken;
                } // switch
            } // switch m_eState

            m_oEnumChar.Next();
        } // for
    } // getToken

    // [R]
    private: void restart()
    {
        m_oEnumChar.SyncEnd();

        Posn lStart = 0;
        foreach (
            Buffer::EnumCharRev,
            oEnum,
            Buffer::EnumCharRev::Arg(m_pBuffer, m_oChange.GetStart()) )
        {
            char16 wch = oEnum.Get();
            if (' ' == wch || 0x09 == wch || 0x0A == wch)
            {
                const StyleValues* pStyle = oEnum.GetStyle();
                if (Syntax_None == pStyle->GetSyntax())
                {
                    lStart = oEnum.GetPosn() - 1;
                    break;
                }
            }
        } // for Buffer::EnumCharRev

        m_eState = State_Normal;

        m_oChange.Reset();

        m_oEnumChar.GoTo(lStart);
    } // restart

    public: bool Run(Count lCount)
    {
        Posn lChange = m_oChange.GetStart();
        if (m_oEnumChar.GetPosn() >= lChange)
        {
            // The buffer is changed since last scan.
            restart();
        }

        m_oEnumChar.SetCounter(lCount);

        while (! m_oEnumChar.AtLimit())
        {
            Token oToken = getToken();
            setColor(
                oToken.m_lStart,
                oToken.m_lEnd,
                oToken.m_eSyntax,
                k_rgnSyntax2Color[oToken.m_eSyntax] );
        } // while

        return ! m_oEnumChar.AtEnd();
    } // Run

    DISALLOW_COPY_AND_ASSIGN(ConfigLexer);
}; // ConfigLexer

const uint32
ConfigLexer::k_rgnSyntax2Color[ConfigLexer::Syntax_Limit] =
{
    RGB(  0,   0,    0),    // Syntax_None
    RGB(  0,  128,   0),    // Syntax_Comment
    RGB(163,   21,  21),    // Syntax_String
    RGB(  0,   0,    0),    // Syntax_Word
}; // k_rgnSyntax2Color

/// <summary>
///   Configuration file mode
/// </summary>
class ConfigMode : public Mode
{
    private: ConfigLexer m_oLexer;

    /// <summary>
    ///   Construct ConfigMode
    /// </summary>
    /// <param naem="pBuffer">A buffer applied to this mode</param>
    /// <param name="pFactory">Mode factory</param>
    public: ConfigMode(ModeFactory* pFactory, Buffer* pBuffer) :
        m_oLexer(pBuffer),
        Mode(pFactory, pBuffer) {}

    // [D]
    /// <summary>
    ///   Color buffer with Config file syntax.
    /// </summary>
    public: virtual bool DoColor(Count lCount) override
    {
        return m_oLexer.Run(lCount);
    } // DoColor

    DISALLOW_COPY_AND_ASSIGN(ConfigMode);
}; // ConfigMode

/// <summary>
///  Construct ConfigModeFactory object
/// </summary>
ConfigModeFactory::ConfigModeFactory() :
    ModeFactory(k_rgnConfigCharSyntax) {}

/// <summary>
///   Create a ConfigModeFactory instance.
/// </summary>
Mode* ConfigModeFactory::Create(Buffer* pBuffer)
{
    return new ConfigMode(this, pBuffer);
} // ConfigModeFactory::Create

/// <summary>
///   Check filename supproted in ConfigMode.
/// </summary>
/// <param name="pwsz">A filename</param>
/// <returns>True if specified filename supported in Config mode</returns>
bool ConfigModeFactory::IsSupported(const char16* pwsz) const
{
    if (0 == lstrcmpW(pwsz, L"Makefile")) return true;
    if (0 == lstrcmpW(pwsz, L"makefile")) return true;
    return ModeFactory::IsSupported(pwsz);
} // ConfigModeFactory::IsSupported

}  // namespace text
