// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/haskell_mode.h"

#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/lexer.h"
#include "evita/text/modes/mode.h"

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// C Keywords
//
static const char16*
k_rgpwszHaskellKeyword[] =
{
    // reservedop
    L"..",  L":",  L"::",  L"=",  L"\\",  L"|",  L"<-",  L"->",
    L"@", L"~", L"=>",

    // Reserved words
    L"case",
    L"class",
    L"data",
    L"default",
    L"deriving",
    L"do",
    L"else",
    L"if",
    L"import",
    L"in",
    L"infix",
    L"infixl",
    L"infixr",
    L"instance",
    L"let",
    L"module",
    L"newtype",
    L"of",
    L"then",
    L"type",
    L"where",
    L"_",

    // import
    L"as",
    L"hiding",
    L"qualified",

    // Well Known Functions
    L"$",
    L"$$",
    L"all",         // PreludeList
    L"and",         // PreludeList
    L"any",         // PreludeList
    L"break",       // PreludeList
    L"catch",
    L"concat",      // PreludeList
    L"const",
    L"curry",
    L"cycle",       // PreludeList
    L"drop",        // PreludeList
    L"dropWhile",   // PreludeList
    L"either",      // Either.either
    L"elem",        // PreludeList
    L"error",
    L"fail",        // Monad.fail
    L"filter",      // PreludeList
    L"fmap",        // Functor.fmap
    L"foldl",       // PreludeList
    L"foldr",       // PreludeList
    L"fst",
    L"head",        // PreludeList
    L"init",        // PreludeList
    L"iterate",     // PreludeList
    L"last",        // PreludeList
    L"length",      // PreludeList
    L"lines",       // PreludeList
    L"lookup",      // PreludeList
    L"map",         // PreludeList
    L"maybe",       // Maybe.maybe
    L"maximum",     // PreludeList
    L"minimum",     // PreludeList
    L"not",
    L"notElem",     // PreludeList
    L"null",        // PreludeList
    L"or",          // PreludeList
    L"otherwise",
    L"product",     // PreludeList
    L"read",
    L"readList",    // PreludeText
    L"readPrec",    // PreludeText
    L"replicate",   // PreludeList
    L"repeat",      // PreludeList
    L"return",      // Mona.return
    L"scanl",       // PreludeList
    L"scanl1",      // PreludeList
    L"scanr",       // PreludeList
    L"seq",
    L"show",
    L"showList",    // PreludeText
    L"showPrec",    // PreludeText
    L"span",        // PreludeList
    L"splitAt",     // PreludeList
    L"sum",         // PreludeList
    L"tail",        // PreludeList
    L"take",        // PreludeList
    L"takeWhile",   // PreludeList
    L"throw",
    L"uncurry",
    L"until",
    L"undefined",
    L"unzip",       // PreludeList
    L"unzip3",      // PreludeList
    L"twords",      // PreludeList
    L"zip",         // PreludeList
    L"zip3",        // PreludeList
    L"zipWith",     // PreludeList
    L"zipWith3",    // PreludeList

    // CPP
    L"#define",
    L"#else",
    L"#elif",
    L"#endif",
    L"#error",
    L"#if",
    L"#ifdef",
    L"#ifndef",
    L"#include",
    L"#line",
    L"#undef",

    // Well Known Types from Prelude
    L"Array",
    L"Bool",
    L"Bounded",
    L"Char",
    L"Data",
    L"Double",
    L"Either",
    L"Enum",
    L"Eq",
    L"False",
    L"Float",
    L"Graph",
    L"Int",
    L"Int8",
    L"Int16",
    L"Int32",
    L"Int64",
    L"Integer",
    L"IO",
    L"IOError",
    L"IORef",
    L"Just",
    L"List",
    L"Maybe",
    L"Monad",
    L"Nothing",
    L"Num",
    L"Ord",
    L"Ordering",
    L"Read",
    L"Show",
    L"String",
    L"Tree",
    L"True",
    L"Tuple",
    L"Word",
    L"Word8",
    L"Word16",
    L"Word32",
    L"Word64",
}; // k_rgpwszHaskellKeyword

// Haskell mode character syntax
static const uint
k_rgnHaskellCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),   // 0x22 "
    CharSyntax::Syntax_Punctuation,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_Punctuation,
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
    CharSyntax::Syntax_Punctuation,                 // 0x5E ^
    CharSyntax::Syntax_Word,                        // 0x5F _

    CharSyntax::Syntax_StringQuote | (0x60 << CharSyntax::Trait_PairShift),   // 0x60 `
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
}; // k_rgnHaskellCharSyntax

/// <summary>
///   A base class of C-like language.
/// </summary>
class HaskellLexer : public NewLexer::LexerBase
{
    public: enum State
    {
        State_StartLine,

        State_Annotation,

        State_BackQuote,

        State_BlockComment,
        State_BlockComment_Star,

        State_Brace,

        State_Dash,
        State_DashDash,

        State_DoubleQuote,
        State_DoubleQuote_Backslash,

        State_LineComment,
        State_LineComment_Backslash,

        State_Normal,

        State_Operator,

        State_Sharp,
        State_SingleQuote,
        State_SingleQuote_Backslash,

        State_Word,

        State_Max_1,
    }; // State

    public: enum Syntax
    {
        Syntax_None,
        Syntax_Comment,
        Syntax_Infix,
        Syntax_Operator,
        Syntax_String,
        Syntax_Word,
        Syntax_WordReserved,

        Syntax_Max_1,
    }; // Syntax

    private: struct Token
    {
        Syntax  m_eSyntax;
        bool    m_fPartial;
        Posn    m_lEnd;
        Posn    m_lStart;
        Posn    m_lWordStart;
        char16  m_wchPrefix;

        Token()
        {
            Reset();
        } // Token

        void Reset(Posn lPosn = 0)
        {
            m_eSyntax    = Syntax_None;
            m_fPartial   = false;
            m_lEnd       = lPosn;
            m_lStart     = lPosn;
            m_lWordStart = lPosn;
            m_wchPrefix  = 0;
        } // Reset
    }; // Token

    private: static const uint32 k_rgnSyntax2Color[Syntax_Max_1];

    private: State          m_eState;
    private: Token          m_oToken;
    private: char16         m_wchAnnotation;

    // ctor
    protected: HaskellLexer(
        Buffer*         pBuffer,
        KeywordTable*   pKeywordTab,
        const uint*     prgnCharSyntax,
        char16          wchAnnotation ) :
            NewLexer::LexerBase(pBuffer, pKeywordTab, prgnCharSyntax),
            m_eState(State_StartLine),
            m_wchAnnotation(wchAnnotation) {}

    private: static KeywordTable* s_pKeywordTab;

    // ctor
    public: HaskellLexer(Buffer* pBuffer) :
        NewLexer::LexerBase(
            pBuffer,
            initKeywordTab(),
            k_rgnHaskellCharSyntax ),
        m_eState(State_StartLine),
        m_wchAnnotation('#') {}

    // [I]
    /// <summary>
    ///   Initialize keyword table.
    /// </summary>
    private: static KeywordTable* initKeywordTab()
    {
        if (NULL == s_pKeywordTab)
        {
            s_pKeywordTab = installKeywords(
                k_rgpwszHaskellKeyword,
                lengthof(k_rgpwszHaskellKeyword) );
        } // if

        return s_pKeywordTab;
    } // HaskellLexer


    // [C]
    /// <summary>
    ///   Color token.
    /// </summary>
    private: void colorToken()
    {
        setColor(
            m_oToken.m_lStart,
            m_oToken.m_lEnd,
            m_oToken.m_eSyntax,
            k_rgnSyntax2Color[m_oToken.m_eSyntax] );
    } // colorToken

    // [E]
    /// <summary>
    ///   Make token complete.
    /// </summary>
    /// <param name="wch">A character ends token.</param>
    private: Syntax endToken(char16 wch)
    {
        m_oToken.m_lEnd = m_oEnumChar.GetPosn();
        m_eState = 0x0A == wch ? State_StartLine : State_Normal;
        return m_oToken.m_eSyntax;
    } // endToken

    // [G]
    /// <summary>
    ///   Get a token.
    /// </summary>
    private: Syntax getToken()
    {
        if (m_oToken.m_fPartial)
        {
            m_oToken.m_fPartial = false;
        }
        else
        {
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
                colorToken();
                return m_oToken.m_eSyntax;
            }

            char16 wch = m_oEnumChar.Get();

          tryAgain:
            switch (m_eState)
            {
            case State_Annotation:
                if (isConsChar(wch))
                {
                    m_oToken.m_eSyntax    = Syntax_Word;
                    m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
                    m_oToken.m_wchPrefix  = m_wchAnnotation;
                    m_eState = State_Word;
                }
                else
                {
                    m_eState = State_Normal;
                    goto tryAgain;
                }
                break;

            case State_BackQuote:
                switch (wch)
                {
                case '`':
                    m_oEnumChar.Next();
                    return endToken(wch);
                }
                break;

            case State_BlockComment:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '-':
                    m_eState = State_BlockComment_Star;
                    break;
                } // swtich wch
                break;

            case State_BlockComment_Star:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '-':
                    m_eState = State_BlockComment_Star;
                    break;

                case '}':
                    m_oEnumChar.Next();
                    return endToken(wch);

                default:
                    m_eState = State_BlockComment;
                    break;
                } // switch wch
                break;

            case State_Brace:
                switch (wch)
                {
                case '-':
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_BlockComment;
                    break;

                default:
                    return endToken(wch);
                } // switch wch
                break;

            case State_Dash:
                switch (wch)
                {
                case '-':
                    m_eState = State_DashDash;
                    break;

                default:
                    m_oToken.m_eSyntax = Syntax_Operator;
                    return endToken(wch);
                } // switch wch
                break;

            case State_DashDash:
                if (isPunctChar(wch))
                {
                    m_oToken.m_eSyntax = Syntax_Operator;
                    return endToken(wch);
                }

                m_oToken.m_eSyntax = Syntax_Comment;
                m_eState = State_LineComment;
                goto tryAgain;

            case State_DoubleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '"':
                    m_oEnumChar.Next();
                    return endToken(wch);

                case '\\':
                    m_eState = State_DoubleQuote_Backslash;
                    break;
                } // switch wc
                break;

            case State_DoubleQuote_Backslash:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                m_eState = State_DoubleQuote;
                break;

            case State_LineComment:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case 0x0A:
                    // We found end of line comment.
                    return endToken(wch);

                case '\\':
                    m_eState = State_LineComment_Backslash;
                    break;
                } // switch wch
                break;

            case State_LineComment_Backslash:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                m_eState = State_LineComment;
                break;

            case State_Normal:
                if (0x0A == wch)
                {
                    m_eState = State_StartLine;
                    break;
                }

                if (0x09 == wch || ' ' == wch)
                {
                    break;
                }

                setColor(
                    m_oToken.m_lStart,
                    m_oEnumChar.GetPosn(),
                    Syntax_None,
                    k_rgnSyntax2Color[Syntax_None] );

                m_oToken.m_lStart = m_oEnumChar.GetPosn();

                switch (wch)
                {
                case '"':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_DoubleQuote;
                    break;

                case '\'':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_SingleQuote;
                    break;

                case '-':
                    m_oToken.m_eSyntax = Syntax_Operator;
                    m_eState = State_Dash;
                    break;

                case '`':
                    m_oToken.m_eSyntax = Syntax_Infix;
                    m_eState = State_BackQuote;
                    break;

                case '{':
                    m_oToken.m_eSyntax = Syntax_Operator;
                    m_eState = State_Brace;
                    break;

                default:
                    if (m_wchAnnotation == wch)
                    {
                        m_eState = State_Annotation;
                    }
                    else if (isConsChar(wch))
                    {
                        m_oToken.m_eSyntax = Syntax_Word;
                        m_oToken.m_lWordStart = m_oToken.m_lStart;
                        m_oToken.m_wchPrefix = 0;
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_oToken.m_eSyntax = Syntax_Operator;
                        m_oToken.m_lWordStart = m_oToken.m_lStart;
                        m_oToken.m_wchPrefix = 0;
                        m_eState = State_Operator;
                    }
                    break;
                } // swtich wch
                break;

            case State_Operator:
                ASSERT(Syntax_Operator == m_oToken.m_eSyntax);
                if (! isPunctChar(wch))
                {
                    return endToken(wch);
                } // if
                break;

            case State_Sharp:
                switch (wch)
                {
                case 0x0A:
                    m_eState = State_StartLine;
                    break;

                case 0x09:
                case 0x20:
                    break;

                default:
                    if (isConsChar(wch))
                    {
                        m_oToken.m_wchPrefix = '#';
                        m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_eState = State_Normal;
                    }
                    break;
                } // swtich wch
                break;

            case State_SingleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '\'':
                    m_oEnumChar.Next();
                    return endToken(wch);

                case '\\':
                    m_eState = State_SingleQuote_Backslash;
                    break;
                } // switch wc
                break;

            case State_SingleQuote_Backslash:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                m_eState = State_SingleQuote;
                break;

            case State_StartLine:
                switch (wch)
                {
                case 0x09:
                case 0x0A:
                case 0x20:
                    break;

                case '#':
                    m_oToken.m_eSyntax = Syntax_Word;
                    m_eState = State_Sharp;
                    break;

                default:
                    m_eState = State_Normal;
                    goto tryAgain;
                } // swtich wch
                break;

            case State_Word:
                ASSERT(Syntax_Word == m_oToken.m_eSyntax);
                if (isConsChar(wch) || wch == '\'')
                {
                    // still collect word
                }
                else
                {
                    return endToken(wch);
                } // if
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch eState

            m_oEnumChar.Next();
        } // for
    } // getToken

    // [P]
    /// <summary>
    ///   Checks token is keyword or not.
    /// </summary>
    private: void processToken()
    {
        if (m_oToken.m_fPartial)
        {
            return;
        }

        switch (m_oToken.m_eSyntax)
        {
        case Syntax_Operator:
        case Syntax_Word:
            break;

        default:
            return;
        } // switch syntax

        char16 wszWord[40];

        int cwchWord = m_oToken.m_lEnd - m_oToken.m_lWordStart + 1;
        if (cwchWord >= lengthof(wszWord))
        {
            return;
        } // if too long word

        char16* pwszWord = wszWord;
        if (0 == m_oToken.m_wchPrefix)
        {
            pwszWord++;
            cwchWord -= 1;
        }
        else
        {
            *pwszWord = m_oToken.m_wchPrefix;
        }

        m_pBuffer->GetText(
            wszWord + 1,
            m_oToken.m_lWordStart,
            m_oToken.m_lEnd );

        StringKey oWord(pwszWord, cwchWord);
        int* piType = m_pKeywordTab->Get(&oWord);
        if (NULL != piType)
        {
            m_oToken.m_eSyntax = Syntax_WordReserved;
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

        m_eState   = State_StartLine;

        Posn lStart = 0;
        foreach (
            Buffer::EnumCharRev,
            oEnum,
            Buffer::EnumCharRev::Arg(m_pBuffer, m_oChange.GetStart()) )
        {
            char16 wch = oEnum.Get();
            if (0x20 == wch || 0x09 == wch || 0x0A == wch)
            {
                const StyleValues* pStyle = oEnum.GetStyle();
                if (Syntax_None == pStyle->GetSyntax())
                {
                    lStart = oEnum.GetPosn() - 1;
                    if (0x0A != wch)
                    {
                        m_eState = State_Normal;
                    }
                    break;
                }
            }
        } // for

        DEBUG_PRINTF("restart from %d state=%d\n",
            lStart, m_eState );

        m_oToken.Reset(lStart);

        m_oChange.Reset();

        m_oEnumChar.GoTo(lStart);
    } // restart

    /// <summary>
    ///   Lexer entry point.
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

    DISALLOW_COPY_AND_ASSIGN(HaskellLexer);
}; // HaskellLexer

//////////////////////////////////////////////////////////////////////
//
// Map Token Type To color
//
const uint32
HaskellLexer::k_rgnSyntax2Color[HaskellLexer::Syntax_Max_1] =
{
    RGB(  0,    0,   0),    // Syntax_None
    RGB(  0,  128,   0),    // Syntax_Comment
    RGB(  0,  128, 255),    // Syntax_Infix
    RGB(  0,    0, 128),    // Syntax_Operator
    RGB(163,   21,  21),    // Syntax_String
    RGB(  0,    0,   0),    // Syntax_Word
    RGB(  0,    0, 255),    // Syntax_WordReserved
}; // k_rgnSyntax2Color

KeywordTable* HaskellLexer::s_pKeywordTab;

/// <summary>
///   Haskell mode
/// </summary>
class HaskellMode : public Mode
{
    private: HaskellLexer m_oLexer;

    // ctor/dtor
    public: HaskellMode(ModeFactory* pFactory, Buffer* pBuffer) :
        m_oLexer(pBuffer),
        Mode(pFactory, pBuffer) {}

    // [D]
    public: virtual bool DoColor(Count lCount) override
    {
        return m_oLexer.Run(lCount);
    } // DoColor

    DISALLOW_COPY_AND_ASSIGN(HaskellMode);
}; // HaskellMode

/// <summary>
///  Construct HaskellModeFactory object
/// </summary>
HaskellModeFactory::HaskellModeFactory() {
}

/// <summary>
///   Construct HaskellMode object.
/// </summary>
Mode* HaskellModeFactory::Create(Buffer* pBuffer)
{
    return new HaskellMode(this, pBuffer);
} // HaskellModeFactory::Create

}  // namespace text
