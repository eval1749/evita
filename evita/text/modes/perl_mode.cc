// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/perl_mode.h"

#include <memory>

#include "common/memory/singleton.h"
#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/lexer.h"
#include "evita/text/modes/mason_mode.h"
#include "evita/text/modes/mode.h"

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// Perl Keywords
//
// FIXME 2007-07-22 yosi@msn.com Should we color Perl special variables?
static const char16*
k_rgpwszPerlKeyword[] =
{
    // Operators
    L"lt", L"gt", L"le", L"ge",
    L"eq", L"ne", L"cmp",
    L"and", L"not", L"or", L"xor",

    // Syntax
    L"do",          // simple statements
    L"elsif",
    L"else",
    L"eval",        // simple statements
    L"for",
    L"foreach",     // statement modifiers
    L"goto",
    L"if",          // statement modifiers
    L"last",        // loop control
    L"my",          // declarations
    L"next",        // loop control
    L"redo",        // loop control
    L"sub",         // declarations
    L"undef",       // declarations
    L"unless",      // statement modifiers
    L"until",       // statement modifiers
    L"while",       // statement modifiers

    // Functions
    // [A]
    L"abs", L"accept", L"alarm", L"atan2",

    // [B]
    L"bind", L"binmod", L"bless",

    // [C]
    L"caller", L"chdir", L"chmod", L"chomp", L"chop", L"chown", L"chr",
    L"chroot", L"close", L"closedir", L"connect", L"continue", L"cos",
    L"ctrypt",

    // [D]
    L"dbmclose", L"dbmopen", L"defined", L"delete", L"die", L"do", L"dump",

    // [E]
    L"each", L"endgrent", L"endhostent", L"endnetent", L"endprotoent",
    L"endpwent", L"endservent", L"eof", L"eval", L"exec", L"exit", L"exp",

    // [F]
    L"fcntl", L"fileno", L"flock", L"fork", L"format", L"formline",

    // [G]
    L"getc", L"getgrent", L"getgrgid", L"getgrnam", L"gethostbyaddr",
    L"gethostent", L"getlogin", L"getnetbyaddr", L"getnetbyname",
    L"getnetent", L"getpeername", L"getpgrp", L"getppid", L"getpriority",
    L"getprotobyname", L"getprotobynumber", L"getprotoent", L"getpwent",
    L"getpwnam", L"getpwuid", L"getservbyname", L"getservbyport",
    L"getservent", L"getsockname", L"getsockopt", L"glob", L"gmtime",
    L"goto", L"grep",

    // [H]
    L"hex",

    // [I]
    L"import", L"index", L"int", L"ioctl",

    // [J]
    L"join",

    // [K]
    L"keys", L"kill",

    // [L]
    L"last", L"lc", L"lcfirst", L"length", L"link", L"lisen", L"local",
    L"localtime", L"lock", L"log", L"lstat",

    // [M]
    L"m", L"map", L"mkdir", L"msgctl", L"msgget", L"msgrcv", L"msgsnd",
    L"my",

    // [N]
    L"next", L"no",

    // [O]

    L"oct", L"open", L"opendir", L"ord", L"our",

    // [P]
    L"pack", L"package", L"pipe", L"pop", L"pos", L"print", L"printf",
    L"prototype", L"push",

    // [Q]
    L"q", L"qq", L"qr", L"quotemeta", L"qw", L"qx",

    // [R]
    L"rand", L"read", L"readdir", L"readline", L"readlink", L"readpipe",
    L"recv", L"redo", L"ref", L"rename", L"require", L"reset", L"return",
    L"reverse", L"rewinddir", L"rindex", L"rmdir",

    // [S]
    L"s", L"scalar", L"seek", L"seekdir", L"select", L"semctl", L"semget",
    L"semop", L"send", L"setgrent", L"sethostent", L"setnetent", L"setpgrp",
    L"setpriority", L"setprotoent", L"setpwent", L"setervent", L"setsockopt",
    L"shift", L"shmctl", L"shmget", L"shmread", L"shmwrite", L"shutdown",
    L"sin", L"sleep", L"socket", L"socketpair", L"sort", L"splice",
    L"split", L"sprintf", L"sqrt", L"srand", L"stat", L"study", L"sub",
    L"substr", L"symlink", L"syscall", L"sysopen", L"sysread", L"sysseek",
    L"system", L"syswrite",

    // [T]
    L"tell", L"telldir", L"tie", L"tied", L"time", L"times", L"tr",
    L"truncate",

    // [U]
    L"uc", L"ucfirst", L"umask", L"undef", L"unlink", L"unpack", L"unshift",
    L"untie", L"use", L"utime",

    // [V]
    L"values", L"vec",

    // [W]
    L"wait", L"waitpid", L"wantarray", L"warn", L"write",

    // [Y]
    L"y",
}; // k_rgpwszPerlKeyword

// Cxx mode character syntax
static const uint
k_rgnPerlCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),  // 0x22 "
    CharSyntax::Syntax_LineComment,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_StringQuote | (0x27 << CharSyntax::Trait_PairShift),  // 0x27 '
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
    CharSyntax::Syntax_Escape,                      // 0x5C backslash(\)
    CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),   // 0x5D ]
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
    CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),   // 0x7B {
    CharSyntax::Syntax_Word,                        // 0x7C |
    CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),   // 0x7D }
    CharSyntax::Syntax_Word,                        // 0x7E ^
    CharSyntax::Syntax_Control,                     // 0x7F DEL
}; // k_rgnPerlCharSyntax

namespace {
class PerlKeywordTable : public common::Singleton<PerlKeywordTable>,
                         public NewLexer::KeywordTable {
  DECLARE_SINGLETON_CLASS(PerlKeywordTable);

  private: PerlKeywordTable() {
    AddKeywords(k_rgpwszPerlKeyword, arraysize(k_rgpwszPerlKeyword));
  }
  public: ~PerlKeywordTable() = default;

  DISALLOW_COPY_AND_ASSIGN(PerlKeywordTable);
};
}  // namespace

/// <summary>
///   A base class of C-like language.
/// </summary>
class PerlLexer : public NewLexer::LexerBase
{
    public: enum State
    {
        State_Normal,

        State_DoubleQuote,
        State_DoubleQuote_Backslash,

        State_LineComment,
        State_LineComment_Backslash,

        State_Number,

        State_SingleQuote,
        State_SingleQuote_Backslash,

        State_Variable,
        State_Variable_Name,

        State_Word,

        State_Max_1,
    }; // State

    public: enum Syntax
    {
        Syntax_None,
        Syntax_Comment,
        Syntax_Number,
        Syntax_Operator,
        Syntax_String,
        Syntax_Variable,
        Syntax_Word,
        Syntax_WordBare,
        Syntax_WordReserved,

        Syntax_Max_1,
    }; // Syntax

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

    private: static const uint32 k_rgnSyntax2Color[Syntax_Max_1];

    private: State          m_eState;
    private: Token          m_oToken;

    // ctor
    public: PerlLexer(
        Buffer*         pBuffer ) :
            NewLexer::LexerBase(
                pBuffer, 
                PerlKeywordTable::instance(), 
                k_rgnPerlCharSyntax ),
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

    // [E]
    private: Syntax endToken()
    {
        m_oToken.m_lEnd = m_oEnumChar.GetPosn();
        m_eState = State_Normal;
        return m_oToken.m_eSyntax;
    } // endToken

    // [G]
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

            switch (m_eState)
            {
            case State_DoubleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '"':
                    m_oEnumChar.Next();
                    return endToken();

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
                    return endToken();

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
                if (0x09 == wch || ' ' == wch || 0x0A == wch)
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

                case '#':
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_LineComment;
                    break;

                case '$':
                case '%':
                case '@':
                case '>':   // for $foo->bar()
                    m_oToken.m_eSyntax = Syntax_Variable;
                    m_eState = State_Variable;
                    break;

                default:
                    if (wch >= '0' && wch <= '9')
                    {
                        m_oToken.m_eSyntax = Syntax_Number;
                        m_eState = State_Number;
                    }
                    else if (isConsChar(wch))
                    {
                        m_oToken.m_eSyntax = Syntax_WordBare;
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_oEnumChar.Next();
                        m_oToken.m_eSyntax = Syntax_Operator;
                        return endToken();
                    }
                    break;
                } // swtich wch
                break;

            case State_Number:
                if (! isConsChar(wch))
                {
                    return endToken();
                }
                break;

            case State_SingleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '\'':
                    m_oEnumChar.Next();
                    return endToken();

                case '\\':
                    m_eState = State_SingleQuote_Backslash;
                    break;
                } // switch wc
                break;

            case State_SingleQuote_Backslash:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                m_eState = State_SingleQuote;
                break;

            case State_Variable:
                ASSERT(Syntax_Variable == m_oToken.m_eSyntax);
                if (isConsChar(wch))
                {
                    m_eState = State_Variable_Name;
                }
                else
                {
                    return endToken();
                }
                break;

            case State_Variable_Name:
                if (! isConsChar(wch))
                {
                    return endToken();
                }
                break;

            case State_Word:
                ASSERT(Syntax_WordBare == m_oToken.m_eSyntax);

                switch (wch)
                {
                case '-':   // $foo->(...)
                case ':':   // foo::bar
                case '(':   // foo(...)
                    m_oToken.m_eSyntax = Syntax_Word;
                    return endToken();

                default:
                    if (! isConsChar(wch))
                    {
                        return endToken();
                    } // if
                } // switch wch
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch eState

            m_oEnumChar.Next();
        } // for
    } // getToken

    // [P]
    private: void processToken()
    {
        if (m_oToken.m_fPartial)
        {
            return;
        }

        switch (m_oToken.m_eSyntax)
        {
        case Syntax_Word:
        case Syntax_WordBare:
            break;

        default:
            return;
        } // switch syntax

        auto const kMaxWordLength = 20;
        int cwchWord = m_oToken.m_lEnd - m_oToken.m_lStart;
        if (cwchWord >= kMaxWordLength) {
            return;
        }
        if (IsKeyword(m_pBuffer->GetText(m_oToken.m_lStart, m_oToken.m_lEnd)))
            m_oToken.m_eSyntax = Syntax_WordReserved;
    } // processToken

    // [R]
    /// <summary>
    ///   Find restart position.
    /// </summary>
    private: void restart()
    {
        m_oEnumChar.SyncEnd();

        DEBUG_PRINTF("Backtrack from %d\n", m_oChange.GetStart());

        m_eState   = State_Normal;
        m_oToken.m_fPartial = false;

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
                    break;
                }
            }
        } // for

        DEBUG_PRINTF("restart from %d state=%d\n",
            lStart, m_eState );

        m_oChange.Reset();

        m_oEnumChar.GoTo(lStart);
    } // restart

    /// <summary>
    ///   Lexer entry point.
    /// </summary>
    public: virtual bool Run(Count lCount) override {
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

    DISALLOW_COPY_AND_ASSIGN(PerlLexer);
}; // PerlLexer

const uint32
PerlLexer::k_rgnSyntax2Color[PerlLexer::Syntax_Max_1] =
{
    RGB(  0,   0,   0), // Syntax_None
    RGB(  0, 128,   0), // Syntax_Comment
    RGB(  0,   0,   0), // Syntax_Number
    RGB(  0,   0,  51), // Syntax_Operator
    RGB(163,  21,  21), // Syntax_String
    RGB(  0,  51,  51), // Syntax_Variable
    RGB(  0,   0,   0), // Syntax_Word
    RGB(102,   0,   0), // Syntax_WordBare
    RGB(  0,   0, 255), // Syntax_WordReserved
}; // PerlLexer::k_rgnSyntax2Color


//////////////////////////////////////////////////////////////////////
//
// MasonLexer
//
class MasonLexer : public PerlLexer {
  public: MasonLexer(Buffer* buffer) : PerlLexer(buffer) {
  }
  public: ~MasonLexer() = default;

  DISALLOW_COPY_AND_ASSIGN(MasonLexer);
};

PerlMode::PerlMode() {
}

PerlMode::~PerlMode() {
}

// Mode
const char16* PerlMode::GetName() const {
  return L"Perl";
}

// ModeWithLexer
Lexer* PerlMode::CreateLexer(Buffer* buffer) {
  return new PerlLexer(buffer);
}

Lexer* MasonMode::CreateLexer(Buffer* buffer) {
  return new MasonLexer(buffer);
}

}  // namespace text
