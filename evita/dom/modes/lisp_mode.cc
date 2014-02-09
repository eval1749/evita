// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/lisp_mode.h"

#include "evita/dom/modes/char_syntax.h"
#include "evita/dom/modes/lexer.h"
#include "evita/dom/modes/mode.h"

namespace text
{

// Common Lisp Keywords
static const char16*
k_rgpwszClKeyword[] =
{
    #include "evita/mode/cl_keywords.inc"

    L"#\\Backspace",
    L"#\\LineFeed",
    L"#\\Newline",
    L"#\\Page",
    L"#\\Return",
    L"#\\Rubount",
    L"#\\Space",
    L"#\\Tab",
}; // k_rgpwszClKeyword

// Cxx mode character syntax
static const uint
k_rgnClCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Word,                        // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),   // 0x22 "
    CharSyntax::Syntax_Punctuation,                 // 0x23 #
    CharSyntax::Syntax_Word,                        // 0x24 $
    CharSyntax::Syntax_Word,                        // 0x25 %
    CharSyntax::Syntax_Word,                        // 0x26 &
    CharSyntax::Syntax_Punctuation,                 // 0x27 '
    CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),   // 0x28 (
    CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),   // 0x29 )
    CharSyntax::Syntax_Word,                        // 0x2A *
    CharSyntax::Syntax_Word,                        // 0x2B +
    CharSyntax::Syntax_Punctuation,                 // 0x2C ,
    CharSyntax::Syntax_Word,                        // 0x2D -
    CharSyntax::Syntax_Word,                        // 0x2E .
    CharSyntax::Syntax_Word,                        // 0x2F /

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
    CharSyntax::Syntax_LineComment,                 // 0x3B ;
    CharSyntax::Syntax_Word,                        // 0x3C <
    CharSyntax::Syntax_Word,                        // 0x3D =
    CharSyntax::Syntax_Word,                        // 0x3E >
    CharSyntax::Syntax_Word,                        // 0x3F ?

    CharSyntax::Syntax_Word,                        // 0x40 @
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
}; // k_rgnClCharSyntax

/// <summary>
///  Lexer for Config file
/// </summary>
class LispLexer : public NewLexer::LexerBase
{
    private: enum State
    {
        State_Normal,

        State_Backslash,
        State_Bar,
        State_Bar_Backslash,
        State_BlockComment,
        State_BlockComment_Bar,
        State_LineComment,
        State_Sharp,
        State_Sharp_Backslash,
        State_Sharp_Backslash_Char,
        State_String,
        State_StringEscape,
        State_Word,
    }; // State

    private: enum Syntax
    {
        Syntax_None,
        Syntax_Comment,
        Syntax_Operator,
        Syntax_String,
        Syntax_Word,
        Syntax_WordQuoted,
        Syntax_WordReserved,
        Syntax_WordSpecial,

        Syntax_Max_1,
    }; // Sytnax

    private: struct Token
    {
        Syntax  m_eSyntax;
        bool    m_fPartial;
        Posn    m_lEnd;
        Posn    m_lStart;

        Token() :
            m_eSyntax(Syntax_None),
            m_fPartial(false),
            m_lEnd(0),
            m_lStart(0) {}
    }; // Token

    /// <summary>
    ///   Map from syntax to color.
    /// </summary>
    private: static const uint32
        k_rgnSyntax2Color[Syntax_Max_1];

    private: State          m_eState;
    private: Token          m_oToken;

    /// <summary>
    ///   Construct Lisp lexer
    /// </summary>
    public: LispLexer(
        Buffer*         pBuffer,
        KeywordTable*   pKeywordTab,
        const uint*     prgnCharSyntax ) :
            NewLexer::LexerBase(
                pBuffer,
                pKeywordTab,
                prgnCharSyntax ),
            m_eState(State_Normal) {}

    // [C]
    private: void colorToken()
    {
        setColor(
            m_oToken.m_lStart,
            m_oToken.m_lEnd,
            m_oToken.m_eSyntax,
            k_rgnSyntax2Color[m_oToken.m_eSyntax] );
    } // colorToken

    // [G]
    /// <summary>
    ///   Get token from buffer.
    /// </summary>
    private: void getToken()
    {
        if (m_oToken.m_fPartial)
        {
            m_oToken.m_fPartial = false;
        }
        else
        {
            m_eState = State_Normal;

            m_oToken.m_eSyntax = Syntax_None;
            m_oToken.m_lStart  = m_oEnumChar.GetPosn();
            m_oToken.m_lEnd    = m_oEnumChar.GetPosn();
        }

        for (;;)
        {
            if (m_oEnumChar.AtLimit())
            {
                // We reached at end of scanning area.
                m_oToken.m_fPartial = true;
                m_oToken.m_lEnd = m_oEnumChar.GetPosn() + 1;
                return;
            }

            char16 wch = m_oEnumChar.Get();

          tryAgain:
            switch (m_eState)
            {
            case State_Backslash:
                m_eState = State_Word;
                break;

            case State_Bar:
                switch (wch)
                {
                case '\\':
                    m_eState = State_Bar_Backslash;
                    break;

                case '|':
                    m_oEnumChar.Next();
                    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
                    return;
                } // switch wch
                break;

            case State_Bar_Backslash:
                m_eState = State_Bar;
                break;

            case State_BlockComment:
                switch (wch)
                {
                case '|':
                    m_eState = State_BlockComment_Bar;
                    break;
                } // switch wch
                break;

            case State_BlockComment_Bar:
                switch (wch)
                {
                case '#':
                    m_oEnumChar.Next();
                    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
                    return;

                default:
                    m_eState = State_BlockComment;
                    goto tryAgain;
                } // switch wch
                break;

            case State_LineComment:
                switch (wch)
                {
                case 0x0A:
                    // We found end of line comment.
                    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
                    m_oEnumChar.Next();
                    m_eState = State_Normal;
                    return;
                } // switch wch
                break;

            case State_Normal:
                if (0x09 == wch || 0x0A == wch || ' ' == wch)
                {
                    break;
                }

                setColor(
                    m_oToken.m_lStart,
                    m_oEnumChar.GetPosn(),
                    Syntax_None,
                    k_rgnSyntax2Color[Syntax_None] );

                m_oToken.m_lStart  = m_oEnumChar.GetPosn();

                switch (wch)
                {
                case '"':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_String;
                    break;

                case '#':
                    m_eState = State_Sharp;
                    break;

                case 0x27:
                case ',':
                    m_oToken.m_eSyntax = Syntax_WordQuoted;
                    m_eState = State_Word;
                    break;

                case ';':
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_LineComment;
                    break;

                case '\\':
                    m_eState = State_Backslash;
                    break;

                case '|':
                    m_oToken.m_eSyntax = Syntax_WordQuoted;
                    m_eState = State_Bar;
                    break;

                default:
                    if (isConsChar(wch) || ':' == wch)
                    {
                        m_oToken.m_eSyntax = Syntax_Word;
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_oToken.m_eSyntax = Syntax_Operator;
                        m_oEnumChar.Next();
                        m_oToken.m_lEnd = m_oEnumChar.GetPosn();
                        m_eState = State_Normal;
                        return;
                    }
                    break;
                } // switch wch
                break;

            case State_Sharp:
                switch (wch)
                {
                case '|':   // #| ... |#
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_BlockComment;
                    break;

                case '\\':  // #\c
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_Sharp_Backslash_Char;
                    break;

                default:
                    m_eState = State_Normal;
                    goto tryAgain;
                } // switch wch
                break;

            case State_Sharp_Backslash_Char:
                m_eState = State_Word;
                break;

            case State_String:
                switch (wch)
                {
                case '"':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_oToken.m_lEnd    = m_oEnumChar.GetPosn() + 1;
                    m_oEnumChar.Next();
                    m_eState = State_Normal;
                    return;

                case '\\':
                    m_eState = State_StringEscape;
                    break;
                } // switch wch
                break;

            case State_StringEscape:
                m_eState = State_String;
                break;

            case State_Word:
                if (isConsChar(wch) || ':' == wch)
                {
                    // symbol name
                }
                else
                {
                    m_oToken.m_eSyntax = Syntax_Word;
                    m_oToken.m_lEnd = m_oEnumChar.GetPosn();
                    m_eState = State_Normal;
                    return;
                } // if
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch m_eState

            m_oEnumChar.Next();
        } // for
    } // getToken

    // [P]
    private: void processNotKeyword()
    {
        if (m_oToken.m_lEnd - m_oToken.m_lStart == 1)
        {
            return;
        }

        char16 wch = m_pBuffer->GetCharAt(m_oToken.m_lStart);
        switch (wch)
        {
        case '#':
            m_oToken.m_eSyntax = Syntax_String;
            break;

        case 0x27:
        case ',':
        case ':':
            m_oToken.m_eSyntax = Syntax_WordQuoted;
            break;

        case '*':
            if ('*' == m_pBuffer->GetCharAt(m_oToken.m_lEnd - 1))
            {
                m_oToken.m_eSyntax = Syntax_WordSpecial;
            }
            break;

        default:
            if ('*' == m_pBuffer->GetCharAt(m_oToken.m_lEnd - 1))
            {
                bool fColon = false;
                foreach (
                    Buffer::EnumChar,
                    oEnum,
                    Buffer::EnumChar::Arg(
                        m_pBuffer,
                        m_oToken.m_lStart,
                        m_oToken.m_lEnd ))
                {
                    char16 wch = oEnum.Get();
                    if (fColon)
                    {
                        switch (wch)
                        {
                        case '*':
                            m_oToken.m_eSyntax = Syntax_WordSpecial;
                            return;

                        case ':':
                            break;

                        default:
                            break;
                        } // switch wch
                    }
                    else
                    {
                        switch (wch)
                        {
                        case ':':
                            fColon = true;
                            break;
                        } // switch wch
                    }
                } // for each char
            } // if end with '*'
            break;
        } /// switch wch
    } // processNotKeyword

    private: void processToken()
    {
        if (Syntax_Word != m_oToken.m_eSyntax)
        {
            return;
        }

        if (m_oToken.m_fPartial)
        {
            return;
        }

        // Note: The longest Common Lisp keyword is
        //  least-negative-normalized-double-float
        char16 wszWord[40];

        int cwchWord = m_oToken.m_lEnd - m_oToken.m_lStart;
        if (cwchWord >= lengthof(wszWord))
        {
            processNotKeyword();
            return;
        } // if too long word

        m_pBuffer->GetText(wszWord, m_oToken.m_lStart, m_oToken.m_lEnd);

        if (':' == *wszWord)
        {
            m_oToken.m_eSyntax = Syntax_WordQuoted;
            return;
        }

        const char16* pwszWord = wszWord;
        if (cwchWord >= 4 &&
            ':' == wszWord[2] &&
            'l' == wszWord[1] &&
            'c' == wszWord[0] )
        {
            pwszWord += 3;
            cwchWord -= 3;
        }

        StringKey oWord(pwszWord, cwchWord);
        int* piType = m_pKeywordTab->Get(&oWord);
        if (NULL != piType)
        {
            m_oToken.m_eSyntax = Syntax_WordReserved;
        }
        else
        {
            processNotKeyword();
        }
    } // processToken

    // [R]
    /// <summary>
    ///   Find restart position.
    /// </summary>
    private: void restart()
    {
        m_oEnumChar.SyncEnd();

        DEBUG_PRINTF("Backtrack from %d\n", m_oChange.GetStart());

        m_eState  = State_Normal;
        m_oToken.m_fPartial = false;

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
        } // for

        DEBUG_PRINTF("restart from %d\n", lStart);

        m_oToken.m_fPartial = false;

        m_oChange.Reset();

        m_oEnumChar.GoTo(lStart);
    } // restart

    /// <summary>
    ///   Lisp lexer entry point.
    /// </summary>
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
            getToken();
            processToken();
            colorToken();
        } // while

        return ! m_oEnumChar.AtEnd();
    } // Run

    DISALLOW_COPY_AND_ASSIGN(LispLexer);
}; // LispLexer

// Map Token Type To color
const uint32
LispLexer::k_rgnSyntax2Color[LispLexer::Syntax_Max_1] =
{
    RGB(  0,   0,    0),    // Syntax_None
    RGB(  0,  128,   0),    // Syntax_Comment
    //RGB(194,    0, 198),    // Syntax_Escaped
    RGB(  0,    0, 198),    // Syntax_Operator
    RGB(163,   21,  21),    // Syntax_String
    RGB(  0,    0,   0),    // Syntax_Word
    RGB(194,    0, 194),    // Syntax_WordQuoted
    RGB(  0,    0, 255),    // Syntax_WordReserved
    RGB(194,    0,   0),    // Syntax_WordSpecial
}; // k_rgnSyntax2Color

/// <summary>
///   Common Lisp lexer
/// </summary>
class ClLexer : public LispLexer
{
    private: static KeywordTable* s_pKeywordTab;

    public: ClLexer(Buffer* pBuffer) :
        LispLexer(
            pBuffer,
            initKeywords(),
            k_rgnClCharSyntax ) {}

    private: static KeywordTable* initKeywords()
    {
        if (NULL == s_pKeywordTab)
        {
            s_pKeywordTab = installKeywords(
                k_rgpwszClKeyword,
                lengthof(k_rgpwszClKeyword) );
        }
        return s_pKeywordTab;
    } // initKeywords

    DISALLOW_COPY_AND_ASSIGN(ClLexer);
}; // ClLexer

KeywordTable* ClLexer::s_pKeywordTab;

/// <summary>
///   Lisp file mode
/// </summary>
class LispMode : public Mode
{
    private: ClLexer m_oLexer;

    /// <summary>
    ///   Construct LispMode
    /// </summary>
    /// <param naem="pBuffer">A buffer applied to this mode</param>
    /// <param name="pFactory">Mode factory</param>
    public: LispMode(ModeFactory* pFactory, Buffer* pBuffer) :
        Mode(pFactory, pBuffer),
        m_oLexer(pBuffer) {}

    // [D]
    /// <summary>
    ///   Color buffer with Lisp syntax.
    /// </summary>
    public: virtual bool DoColor(Count lCount) override
    {
        return m_oLexer.Run(lCount);
    } // DoColor

    DISALLOW_COPY_AND_ASSIGN(LispMode);
}; // LispMode

/// <summary>
///  Construct LispModeFactory object
/// </summary>
LispModeFactory::LispModeFactory() :
    ModeFactory(k_rgnClCharSyntax) {}

/// <summary>
///   Create a LispModeFactory instance.
/// </summary>
Mode* LispModeFactory::Create(Buffer* pBuffer)
{
    return new LispMode(this, pBuffer);
} // LispModeFactory::Create

}  // namespace text
