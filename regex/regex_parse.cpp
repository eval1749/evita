#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Regex - Parser
// regex/regex_parse.cpp
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_parse.cpp#11 $
//
#include "./IRegex.h"
#include "./regex_node.h"

namespace Regex
{

namespace RegexPrivate
{

//////////////////////////////////////////////////////////////////////
//
// k_wszOptions
//  i   IgnoreCase
//  m   Multiline
//  n   ExplicitCapture
//  s   Singleline
//  u   Unicode
//  x   ExtendedSyntax
static const char16
k_wszOptions[] = L"imnsux";

static const char16
k_rgwchBackslashMap[] =
{
    'a',    0x07,   // \a = alert
    'b',    0x08,   // \b = backspace
    'e',    0x1B,   // \e = escape
    'f',    0x0C,   // \f = page
    'n',    0x0A,   // \n = newline(=lf)
    'r',    0x0D,   // \r = caridge return
    't',    0x09,   // \t = tab
    'v',    0x0B,   // \v = vertical tab
    '\\',   0x5C,   // \\ = backslash
}; // k_rgwchBackslashMap

struct OneWidthEntry
{
    char16  m_wch;
    Op      m_eOpAsciiB;
    Op      m_eOpAsciiF;
    Op      m_eOpUnicodeB;
    Op      m_eOpUnicodeF;
}; // OneWidthEntry

#define ONE_WIDTH_ENTRY(mp_ch, mp_class) \
    { \
        mp_ch, \
        Op_Ascii   ## mp_class ## _B, \
        Op_Ascii   ## mp_class ## _F, \
        Op_Unicode ## mp_class ## _B, \
        Op_Unicode ## mp_class ## _F, \
    }

static const OneWidthEntry k_rgoOneWidthMap[] =
{
    ONE_WIDTH_ENTRY('D', DigitCharNe),
    ONE_WIDTH_ENTRY('d', DigitCharEq),
    ONE_WIDTH_ENTRY('S', SpaceCharNe),
    ONE_WIDTH_ENTRY('s', SpaceCharEq),
    ONE_WIDTH_ENTRY('W', WordCharNe),
    ONE_WIDTH_ENTRY('w', WordCharEq),
}; // k_rgoOneWidthMap


struct ZeroWidthEntry
{
    char16  m_wch;
    Op      m_eOpAscii;
    Op      m_eOpUnicode;
}; // ZeroWidthEntry

// \Z end of string or before string-ending newline
static const ZeroWidthEntry k_rgoZeroWidthMap[] =
{
    { 'A',  Op_StartOfString,   Op_StartOfString },
    { 'B',  Op_AsciiNotBoundary,Op_UnicodeNotBoundary },
    { 'b',  Op_AsciiBoundary,   Op_UnicodeBoundary },
    { 'G',  Op_Posn,            Op_Posn },
    { 'Z',  Op_EndOfLine,       Op_EndOfLine },
    { 'z',  Op_EndOfString,     Op_EndOfString },
}; // k_rgoZeroWidthMap

static bool isNameFirstChar(int iChar)
{
    if (iChar >= 'A' && iChar <= 'Z') return true;
    if (iChar >= 'a' && iChar <= 'z') return true;
    return '_' == iChar;
} // isNameFirstChar

static bool isNameChar(int iChar)
{
    if (iChar >= '0' && iChar <= '9') return true;
    return isNameFirstChar(iChar);
} // isNameChar

static int toXdigit(int iChar)
{
    if (iChar >= '0' && iChar <= '9') return iChar - '0';
    if (iChar >= 'A' && iChar <= 'F') return iChar - 'A' + 10;
    if (iChar >= 'a' && iChar <= 'f') return iChar - 'a' + 10;
    return -1;
} // toXdigit

//////////////////////////////////////////////////////////////////////
//
// Parser
//
// Note:
//  isBackward affetcs results of
//      o Lookaround
//      o MultilineDollar
//      o OneWidth
//      o SinglelineDollar
//
class Parser
{
    private: typedef Node::Case Case;
    private: typedef Node::Direction Direction;

    private: enum BackslashFlavor
    {
        BackslashFlavor_RangeMax,
        BackslashFlavor_RangeMin,
        BackslashFlavor_Regex,
    }; // BackslashFlavor

    private: enum TokenType
    {
        TokenType_None,

        TokenType_And,
        TokenType_Any,                  // .
        TokenType_Asterisk,             // r*
        TokenType_Atom,                 // (?>...)
        TokenType_Capture,              // (...)
        TokenType_CloseParen,           // (...)
        TokenType_Eof,
        TokenType_Group,                // (?:...)
        TokenType_Node,
        TokenType_If,                   // (?(cond) then else)
        TokenType_MultiLineDollar,      // (?m:$)
        TokenType_NamedCapture,         // (?<name>...) or (?'name'...)
        TokenType_NegativeLookahead,    // (?! ...)
        TokenType_NegativeLookbehind,   // (?<! ...)
        TokenType_OpenBrace,            // {m,n}
        TokenType_OptionOff,            // (?-imsux:...)
        TokenType_OptionOn,             // (?imsux:...)
        TokenType_Or,                   // r|r
        TokenType_Plus,                 // r+
        TokenType_PositiveLookahead,    // (?= ...)
        TokenType_PositiveLookbehind,   // (?<= ...)
        TokenType_Question,             // r?
        TokenType_SingleLineDollar,
    }; // TokenType

    private: struct Token
    {
        TokenType   m_eType;
        Node*       m_pNode;

        Token(TokenType const eType = TokenType_None) :
            m_eType(eType),
            m_pNode(NULL) {}

        Token(Node* const pNode) :
            m_eType(TokenType_Node),
            m_pNode(pNode) {}
            
        TokenType GetType() const
            { return m_eType; }
    }; // Token

    private: struct CaptureRef :
        LocalObject,
        DoubleLinkedItem_<CaptureRef>
    {
        long                const m_lPosn;
        NodeCaptureBase*    const m_pNode;
        char16*             const m_pwszName;

        public: CaptureRef(
            NodeCaptureBase*    const pNode,
            long                const lPosn,
            char16*             const pwszName = NULL ) :
                m_pNode(pNode), 
                m_lPosn(lPosn), 
                m_pwszName(pwszName) {}
                
        private: CaptureRef& operator=(CaptureRef&)
            { NOTREACHED(); }
    }; // CaptureRef

    private: typedef DoubleLinkedList_<CaptureRef> CaptureRefs;

    private: struct OpenScope
    {
        Parser*       const m_pParser;
        const char16* const m_pwchOpen;

        public: OpenScope(Parser* pParser) :
            m_pParser(pParser),
            m_pwchOpen(pParser->m_pwchOpen) {}

        public: OpenScope(Parser* pParser, const char16* pwch) :
            m_pParser(pParser),
            m_pwchOpen(pParser->m_pwchOpen)
        {
            pParser->m_pwchOpen = pwch;
        } // OpenScope

        public: ~OpenScope()
        {
            m_pParser->m_pwchOpen = m_pwchOpen;
        } // ~OpenScope
        
        private: OpenScope& operator=(OpenScope&)
            { NOTREACHED(); }
    }; // OpenScope

    private: typedef int Char;

    private: int            m_cCaptures;
    private: CaptureRefs    m_oCaptureRefs;
    private: Token          m_oUngetToken;
    private: LocalHeap*     m_pHeap;
    private: IEnvironment*  m_pIEnv;
    private: Tree*          m_pTree;
    private: const char16*  m_pwch;
    private: const char16*  m_pwchEnd;
    private: const char16*  m_pwchOpen;
    private: const char16*  m_pwchStart;
    private: int            m_rgfOption;
    private: int            m_rgfTokenOption;

    // ctor
    public: Parser(
        IEnvironment*   pIEnv,
        LocalHeap*      pHeap,
        const char16*   pwch,
        int             cwch,
        int             rgfOption ) :
            m_cCaptures(0),
            m_pHeap(pHeap),
            m_pIEnv(pIEnv),
            m_pTree(NULL),
            m_pwch(pwch),
            m_pwchEnd(pwch + cwch),
            m_pwchOpen(NULL),
            m_pwchStart(pwch),
            m_rgfOption(rgfOption),
            m_rgfTokenOption(0)
            {}

    // [G]
    private: Char getChar()
    {
        if (! isExtendedSyntax())
        {
            return getCharAux();
        }

        for (;;)
        {
            Char iChar = getCharAux();
            if (Eof == iChar) return iChar;

            if (! IsWhitespace(static_cast<char16>(iChar)))
            {
                return iChar;
            }
        } // for
    } // getChar

    private: Char getCharAux()
    {
        int iChar = m_pwch < m_pwchEnd ? *m_pwch : Eof;
        m_pwch++;
        return iChar;
    } // getCharAux

    private: int getNamedCapture(CharSink* pName)
    {
        OpenScope oScope(this, m_pwch);

        char16 wchOpen = static_cast<char16>(getChar());
        char16 wchClose;
        switch (wchOpen)
        {
        case '<':           // for (?<name> ...) or \k<name>
            wchClose = '>';
            break;
        case OpenParen:     // for (?(name) then | else)
            wchClose = CloseParen;
            break;
        case Quote:         // for (?'name' ...) or \k'name'
            wchClose = Quote;
            break;
        default:
            syntaxError();
            return -1;
        } // switch

        int iChar = getChar();
        if (Eof == iChar)
        {
            unclosedPair(wchOpen);
            return -1;
        }

        if (iChar >= '0' && iChar <= '9')
        {
            int iNum = iChar - '0';
            for (;;)
            {
                int const iChar = getChar();
                if (wchClose == iChar)
                {
                    return iNum;
                }

                if (iChar < '0' || iChar > '9')
                {
                    if (Eof == iChar)
                    {
                        unclosedPair(wchOpen);
                    }
                    else
                    {
                        signalError(Regex::Error_InvalidName);
                    }
                    return -1;
                }

                iNum *= 10;
                iNum += iChar - '0';
            } // for
        }
        else if (isNameFirstChar(iChar))
        {
            for (;;)
            {
                pName->Add(static_cast<char16>(iChar));
                iChar = getChar();

                if (wchClose == iChar)
                {
                    return 0;
                }

                if (Eof == iChar)
                {
                    unclosedPair(wchOpen);
                    return -1;
                }

                if (! isNameChar(iChar))
                {
                    signalError(Regex::Error_InvalidName);
                    return -1;
                }
            } // for
        }
        else
        {
            syntaxError();
            return -1;
        }
    } // getNamedCapture

    private: int getSourcePosn() const
        { return static_cast<int>(m_pwch - m_pwchStart); }

    private: Token getToken()
    {
        if (TokenType_None != m_oUngetToken.GetType())
        {
            Token oToken = m_oUngetToken;
            m_oUngetToken.m_eType = TokenType_None;
            return oToken;
        }

        int iChar = getChar();
        if (Eof == iChar)
        {
            return Token(TokenType_Eof);
        }

        char16 wch = static_cast<char16>(iChar);

        switch (wch)
        {
        case '*':
            return Token(TokenType_Asterisk);

        case '+':
            return Token(TokenType_Plus);

        case '?':
            return Token(TokenType_Question);

        case '.':
            if (isSingleLine())
            {
                return Token(TokenType_Any);
            }

            return Token(newChar(Newline, Node::CaseSensitive, true));

        case '$':
            return Token(isMultiLine() ?
                TokenType_MultiLineDollar :
                TokenType_SingleLineDollar );

        case '^':
            return Token(newZeroWidth(isMultiLine() ?
                Op_AfterNewline :
                Op_StartOfString ) );

        case '|':
            return Token(TokenType_Or);

        case OpenParen:
            m_pwchOpen = m_pwch - 1;
            return getTokenParen();

        case CloseParen:
            return Token(TokenType_CloseParen);

        case OpenBracket:
            m_pwchOpen = m_pwch - 1;
            return parseCharClass();

        case Backslash:
            return getTokenBackslash();

        case OpenBrace:
            return Token(TokenType_OpenBrace);

        case '#':
            if (! isExtendedSyntax())
            {
                return newChar('#');
            }

            // Skip until newline
            for (;;)
            {
                Char iChar = getCharAux();
                if (Eof == iChar || Newline == iChar)
                {
                    return getToken();
                }
            } // for

        default:
            return newChar(wch);
        } // switch ch
    } // getToken

    private: Node* getTokenBackslash()
    {
        Node* pNode =getTokenBackslashAux(BackslashFlavor_Regex);
        if (NULL == pNode)
        {
            pNode = signalError(Regex::Error_InvalidBackslash);
        }
        return pNode;
    } // getTokenBackslash

    //  Returns token for backslash notation outside character class. This
    //  function returns :boundary for "\b" instead of U+0008 (BS).
    //  See Also: cc-get-token/backslash.
    private: Node* getTokenBackslashAux(BackslashFlavor eFlavor)
    {
        int iChar = getChar();
        if (Eof == iChar) return nullptr;

        char16 wch = static_cast<char16>(iChar);

        // Regex metacharacter
        switch (eFlavor)
        {
        case BackslashFlavor_Regex:
            if (NULL != lstrchrW(L".*+?()[]{}\\^$#", wch))
            {
                return newChar(wch);
            }

            // Zero Width Assertion \A \B \b \G \Z \z
            // Note: we must do zero-width assertion test before backslash
            // map, since backslash map contains entry of "\b".
            for (
                const ZeroWidthEntry* p = k_rgoZeroWidthMap;
                p < &k_rgoZeroWidthMap[ARRAYSIZE(k_rgoZeroWidthMap)];
                p++ )
            {
                if (p->m_wch == wch)
                {
                    return newZeroWidth(
                        isUnicode() ? p->m_eOpUnicode : p->m_eOpAscii );
                }
            } // for

            break;

        case BackslashFlavor_RangeMin:
        case BackslashFlavor_RangeMax:
            if ('-' == wch || CloseBracket == wch)
            {
                return newChar(wch);
            }
            break;
        } // switch

        // \D \d \S \s \W \w
        for (
            const OneWidthEntry* p = k_rgoOneWidthMap;
            p < &k_rgoOneWidthMap[ARRAYSIZE(k_rgoOneWidthMap)];
            p++ )
        {
            if (p->m_wch == wch)
            {
                if (BackslashFlavor_RangeMax == eFlavor)
                {
                    return nullptr;
                }

                return newOneWidth(isBackward() ?
                    (isUnicode() ? p->m_eOpUnicodeB : p->m_eOpAsciiB) :
                    (isUnicode() ? p->m_eOpUnicodeF : p->m_eOpAsciiF) );
            }
        } // for

        // \a \b \e \f \n \r \t \v
        for (
            const char16* pwch = k_rgwchBackslashMap;
            pwch < &k_rgwchBackslashMap[ARRAYSIZE(k_rgwchBackslashMap)];
            pwch += 2 )
        {
            if (*pwch == wch) return newChar(pwch[1]);
        } // for

        // \0oo
        if ('0' == wch)
        {
            int iOct = wch - '0';
            int cDigits = 1;
            while (cDigits <= 3)
            {
                int const iChar = getChar();

                if (Eof == iChar)
                {
                    break;
                }

                if (iChar < '0' || iChar > '7')
                {
                    break;
                }

                iOct *= 8;
                iOct += iChar - '0';
                cDigits += 1;
            } // while
            ungetChar();
            return newChar(static_cast<char16>(iOct));
        } // if

        // \<digit>+
        if (wch >= '1' && wch <= '9')
        {
            int iOct = wch - '0';
            int iNum = wch - '9';
            int cDigits = 1;
            bool fOct = wch <= '7';
            while (cDigits <= 3)
            {
                int const iChar = getChar();

                if (Eof == iChar)
                {
                    break;
                }

                if (iChar < '0' || iChar > '9')
                {
                    break;
                }

                if (fOct)
                {
                    fOct = wch <= '7';
                }

                iOct *= 8;
                iOct += iChar - '0';

                iNum *= 10;
                iNum += iChar - '0';

                cDigits += 1;
            } // while

            ungetChar();

            if (! isExplicitCapture() && BackslashFlavor_Regex == eFlavor)
            {
                NodeCaptureEq* const pNode= new(m_pHeap) NodeCaptureEq(
                    getDir(),
                    iNum,
                    getCase() );

                if (iNum > m_cCaptures)
                {
                    CaptureRef* pRef = new(m_pHeap) CaptureRef(
                        pNode,
                        getSourcePosn() );

                    m_oCaptureRefs.Append(pRef);
                } // if

                return pNode;
            } // if

            if (3 == cDigits)
            {
                return newChar(static_cast<char16>(iOct), getCase());
            }
            return nullptr;
        } // if <digit>

        // \xXX or \x{XXXX}
        if ('x' == wch)
        {
            int iChar = getChar();
            if (Eof == iChar)
            {
                return nullptr;
            }

            int iNum = toXdigit(iChar);
            if (iNum >= 0)
            {
                iNum *= 16;
                int iDigit = toXdigit(getChar());

                if (iDigit < 0)
                {
                    return nullptr;
                }

                iNum += iDigit;
                return newChar(static_cast<char16>(iNum));
            } // if

            if (OpenBrace == iChar)
            {
                int cDigits = 0;
                iNum = 0;
                for (;;)
                {
                    int const iChar = getChar();
                    if (CloseBrace == iChar)
                    {
                        if  (cDigits >= 1)
                        {
                            return newChar(static_cast<char16>(iNum));
                        }
                        break;
                    }

                    int iDigit = toXdigit(iChar);
                    if (iDigit < 0)
                    {
                        break;
                    }

                    iNum *= 16;
                    iNum += iDigit;
                    cDigits += 1;
                } // for
            } // if x

            return nullptr;
        } // if x

        // \uXXXX
        if ('u' == wch)
        {
            int cDigits = 0;
            int iNum = 0;
            for (;;)
            {
                int iDigit = toXdigit(getChar());
                if (iDigit < 0)
                {
                    ungetChar();
                    break;
                }
                iNum *= 16;
                iNum += iDigit;
                cDigits += 1;
            } // for

            if (4 != cDigits) return nullptr;

            return newChar(static_cast<char16>(iNum));
        } // if u

        // \\cC
        if ('c' == wch)
        {
            int iChar = getChar();
            if (iChar >= 0x3F && iChar <= 0x5F)
            {
                return newChar(static_cast<char16>(iChar ^ 0x40));
            }

            if (iChar >= 0x60 && iChar <= 0x7A)
            {
                return newChar(static_cast<char16>(iChar - 0x60));
            }

            return nullptr;
        } // if c

        if (BackslashFlavor_Regex != eFlavor) return nullptr;

        if (m_pIEnv->IsUnicodeSpaceChar(wch))
        {
            return newChar(wch);
        } // if space

        // \k = named capture reference
        if ('k' == wch)
        {
            return getTokenNamedCaptureRef();
        } // if k

        // \Q...\E  literal span
        if ('Q' == wch)
        {
            CharSink oSink(m_pHeap);
            char16 wchLast = 0;
            for (;;)
            {
                int const iChar = getChar();

                if (Eof == iChar)
                {
                    return new(m_pHeap) NodeVoid;
                }

                if ('E' == iChar && Backslash == wchLast)
                {
                    return new(m_pHeap) NodeString(
                        getDir(),
                        oSink.GetStart(),
                        oSink.GetLength() );
                }
                wchLast = static_cast<char16>(iChar);
            } // for
        } // if Q

        // \C           force matched single-byte
        // \L ... \E    lower case span
        // \N{name}     Named Unicode character
        // \l char      lower case char, e.g. \l$foo
        // \U ... \E    upper case span
        // \u char      upper case char, e.g. \u$foo
        // \X           Unicode combining character sequence \P{M}\p{M}*
        if (NULL != lstrchrW(L"CLNUXlu", wch))
        {
            return signalError(Regex::Error_NotSupported);
        }

        // Unrecognized backslash
        return nullptr;
    } // getTokenBackslash

    private: Node* getTokenNamedCaptureRef()
    {
        CharSink oName(m_pHeap);

        int const iNth = getNamedCapture(&oName);

        if (iNth < 0)
        {
            return new(m_pHeap) NodeVoid;
        }

        if (0 == iNth)
        {
            CaptureDef* const pCaptureDef =
                findNamedCapture(oName.GetStart());

            if (NULL != pCaptureDef)
            {
                return new(m_pHeap) NodeCaptureEq(
                    getDir(),
                    pCaptureDef->m_iNth,
                    getCase() );
            }
        } // if

        NodeCaptureEq* const pNode = new(m_pHeap) NodeCaptureEq(
            getDir(),
            iNth,
            getCase() );

        if (0 == iNth || iNth > m_cCaptures)
        {
            CaptureRef* const pCaptureRef = new(m_pHeap) CaptureRef(
                pNode,
                getSourcePosn(),
                oName.Save(m_pHeap) );

            m_oCaptureRefs.Append(pCaptureRef);
        } // if

        return pNode;
    } // getTokenNamedCaptureRef

    // getTokenParen
    //  (?! ...)
    //  (?# ...)
    //  (?'name' ...)
    //  (?(...) ...)
    //  (?)
    //  (?-mods: ...)
    //  (?<name> ...)
    //  (?<! ...)
    //  (?<= ...)
    //  (?= ...)
    //  (?> ...)
    //  (??{code})
    //  (?imsux: ...)
    //  (?{code})
    private: Token getTokenParen()
    {
        int iChar = getChar();

        if (Eof == iChar)
        {
            return unexpectedEof();
        }

        if ('?' != iChar)
        {
            ungetChar();
            return Token(TokenType_Capture);
        }

        iChar = getChar();
        if (Eof == iChar)
        {
            return unexpectedEof();
        }

        char16 const wch = static_cast<char16>(iChar);
        switch (iChar)
        {
        case '!':   // (?! ...) = negative Lookahead
            return Token(TokenType_NegativeLookahead);

        case '#':   // (?# comment )
            for (;;)
            {
                int const iChar = getChar();
                if (Eof == iChar)
                {
                    unexpectedEof();
                }

                if (CloseParen == iChar)
                {
                    break;
                }
            } // for
            return getToken();

        case Quote:         // (?'name' ...)
            ungetChar();
            return Token(TokenType_NamedCapture);

        case OpenParen:    // (?(...) ... ...)
            return Token(TokenType_If);

        case CloseParen:    // (?) = empty grouping
            return getToken();

        case '-':   // (?-mods:...)
            return TokenType_OptionOff;

        case ':':   // (?: grouping-only *)
            return Token(TokenType_Group);

        case '<':
            switch (getChar())
            {
            case '!':   // (?!= ...)
                return Token(TokenType_NegativeLookbehind);

            case '=':   // (?<= ...)
                return Token(TokenType_PositiveLookbehind);

            default:    // (?<name> ...)
                ungetChar();
                ungetChar();
                return Token(TokenType_NamedCapture);
            } // switch wch

        case '=':   // (?= ...) = positive Lookahead
            return Token(TokenType_PositiveLookahead);

        case '>':   // (?> atomic-grouping *)
            return Token(TokenType_Atom);

        case '?':   // (??{code})
            return signalError(Regex::Error_NotSupported);

        case OpenBrace:    // (?{code})
            return signalError(Regex::Error_NotSupported);

        default:
            if (NULL != lstrchrW(k_wszOptions, wch))
            {
                // (?mods-mods:...)
                ungetChar();
                return Token(TokenType_OptionOn);
            }
        } // switch
        return signalError(Regex::Error_Syntax);
    } // getTokenParen

    int getUInt()
    {
        int iNum = 0;
        int cDigits = 0;
        for (;;)
        {
            int const iChar = getChar();
            if (iChar < '0' || iChar > '9')
            {
                break;
            }
            cDigits += 1;
            iNum *= 10;
            iNum += iChar - '0';
        } // for
        ungetChar();
        return cDigits > 0 ? iNum : -1;
    } // getUInt

    // [F]
    private: CaptureDef* findNamedCapture(const char16* const pwszName)
    {
        foreach (CaptureDefs::Enum, oEnum, &m_pTree->m_oCaptures)
        {
            CaptureDef* const pCaptureDef = oEnum.Get();
            if (0 == lstrcmpW(pCaptureDef->m_pwszName, pwszName))
            {
                return pCaptureDef;
            }
        } // for
        return nullptr;
    } // findNamedCapture

    // [G]
    private: Case getCase() const
    {
        return isIgnoreCase() ? Node::CaseInsensitive : Node::CaseSensitive;
    } // getCase

    private: Direction getDir() const
        { return isBackward() ? Node::Backward : Node::Forward; }

    private: Direction getReverse() const
        { return isBackward() ? Node::Forward : Node::Backward; }

    // [I]
    private: bool isBackward() const
        { return 0 != (m_rgfOption & Regex::Option_Backward); }

    private: bool isExplicitCapture() const
        { return 0 != (m_rgfOption & Regex::Option_ExplicitCapture); }

    private: bool isExtendedSyntax() const
        { return 0 != (m_rgfOption & Regex::Option_ExtendedSyntax); }

    private: bool isIgnoreCase() const
        { return 0 != (m_rgfOption & Regex::Option_IgnoreCase); }

    private: bool isMultiLine() const
        { return 0 != (m_rgfOption & Regex::Option_Multiline); }

    private: bool isNoCapture() const
        { return 0 != (m_rgfOption & Regex::Option_ExplicitCapture); }

    private: bool isSingleLine() const
        { return 0 != (m_rgfOption & Regex::Option_Singleline); }

    private: bool isUnicode() const
        { return 0 != (m_rgfOption & Regex::Option_Unicode); }

    // [N]
    private: Node* newChar(char16 wch)
    {
        return newChar(wch, getCase());
    } // newChar

    private: Node* newChar(
        char16  const wch,
        Case    const eCase,
        bool    const fNot = false)
    {
        Node* const pNode = new(m_pHeap) NodeChar(
            getDir(),
            wch,
            eCase,
            fNot );
        return pNode->Simplify(m_pIEnv, m_pHeap);
    } // newChar

    private: Node* newOneWidth(Op const eOp)
    {
        Node* pNode = new(m_pHeap) NodeOneWidth(eOp);
        return pNode->Simplify(m_pIEnv, m_pHeap);
    } // newOneWidth

    private: Node* newZeroWidth(Op const eOp)
    {
        Node* pNode = new(m_pHeap) NodeZeroWidth(eOp);
        return pNode->Simplify(m_pIEnv, m_pHeap);
    } // newZeroWidth

    // [P]
    private: Node* parse(TokenType const eEnd)
    {
        int const rgfOption = m_rgfOption;
        Node* const pNode = parseAux(eEnd);
        m_rgfOption = rgfOption;
        return pNode;
    } // parse

    private: void parseAll()
    {
        m_pTree->m_pNode     = parse(TokenType_Eof);
        m_pTree->m_cCaptures = m_cCaptures;

        // Resolve capture reference
        foreach (CaptureRefs::Enum, oEnum, &m_oCaptureRefs)
        {
            CaptureRef* const pRef = oEnum.Get();
            if(pRef->m_pNode->GetNth() > m_cCaptures)
            {
                m_pwch = m_pwchStart + pRef->m_lPosn;
                signalError(Regex::Error_UnboundCapture);
                break;
            } // if

            if (NULL != pRef->m_pwszName)
            {
                CaptureDef* const pDef = findNamedCapture(pRef->m_pwszName);
                if (NULL == pDef)
                {
                    m_pwch = m_pwchStart + pRef->m_lPosn;
                    signalError(Regex::Error_UnboundCapture);
                    break;
                }

                pRef->m_pNode->SetNth(pDef->m_iNth);
            } // if
        } // for each CaptureRef
    } // parseAll

    private: Node* parseAux(TokenType const eEnd)
    {
        Node* const pNode = parseAlt(eEnd);

        Token const oToken = getToken();

        if (oToken.GetType() == eEnd)
        {
            return pNode;
        }

        if (Eof == eEnd)
        {
            return syntaxError();
        }

        return unclosedPair(OpenParen);
    } // parseAux

    // parseAlt -- catexp '|' [altexp]
    private: Node* parseAlt(TokenType const eEnd)
    {
        Node* const pNode = parseCat(eEnd);
        NodeOr* pAltNode = NULL;
        Token oToken;
        for (
            oToken = getToken();
            TokenType_Or == oToken.GetType();
            oToken = getToken() )
        {
            if (NULL == pAltNode)
            {
                pAltNode = new(m_pHeap) NodeOr;
            }

            pAltNode->Append(pNode);
            pAltNode->Append(parseCat(eEnd));
        } // for

        if (oToken.GetType() != TokenType_None)
        {
            ungetToken(oToken);
        }

        return NULL == pAltNode ? pNode : pAltNode;
    } // parseAlt

    // parseCat -- qntexp+
    private: Node* parseCat(TokenType const eEnd)
    {
        Node* pNode = parseQnt(eEnd);
        NodeAnd* pCatNode = NULL;

        for (;;)
        {
            Token const oToken = getToken();
            ungetToken(oToken);
            switch (oToken.GetType())
            {
            case TokenType_CloseParen:
            case TokenType_Eof:
            case TokenType_Or:
                return pNode->Simplify(m_pIEnv, m_pHeap);

            default:
                if (NULL == pCatNode)
                {
                    pCatNode = new(m_pHeap) NodeAnd;
                    pCatNode->Append(pNode);
                    pNode = pCatNode;
                }
                pCatNode->Append(parseQnt(eEnd));
                break;
            } // switch
        } // for
    } // parseCat

    // Note:
    //  A character followed by open brack("[") isn't meta character. So,
    //  following examples are valid:
    //      "[-]"       "-"
    //      "[a-]"      "a" and "-"
    //      "[--A]"     "-" to "A"
    //      "[]]"       "]"
    //      "[]-a]"     "]" to "a"
    //      "[a-a]"     => error, min == max
    //      "[z-a]"     => error, min > max
    private: Node* parseCharClass()
    {
        OpenScope oScope(this, m_pwch - 1);

        bool fNot = false;
        if ('^' == getChar())
        {
            fNot  = true;
        }
        else
        {
            ungetChar();
        }

        NodeCharClass* const pCharClass = new(m_pHeap) NodeCharClass(
            getDir(),
            fNot );

        enum State
        {
            State_Start,

            State_Dash,
            State_Max,
            State_Min,
        } eState = State_Start;

        // Note: dummy initialization for suppressing compiler warning
        char16 wchMin = 0;

        for (;;)
        {
            int const iChar = getChar();
            if (Eof == iChar)
            {
                return unclosedPair(OpenBracket);
            }

            char16 wch = static_cast<char16>(iChar);

            switch (eState)
            {
            case State_Start:
                switch (wch)
                {
                case Backslash:
                    goto handleMinBackslash;

                default:
                    wchMin = wch;
                    eState = State_Dash;
                    break;
                } // switch
                break;

            case State_Dash:
                switch (wch)
                {
                case '-':
                    eState = State_Max;
                    break;

                case Backslash:
                    goto handleMinBackslash;

                case CloseBracket:
                    pCharClass->Append(newChar(wchMin));
                    return pCharClass->Simplify(m_pIEnv, m_pHeap);

                default:
                    pCharClass->Append(newChar(wchMin));
                    wchMin = wch;
                    break;
                } // switch
                break;

            case State_Max:
                switch (wch)
                {
                case '-':
                    return signalError(Regex::Error_InvalidRange);

                case Backslash:
                {
                    Node* const pNode = getTokenBackslashAux(
                        BackslashFlavor_RangeMax );

                    if (NULL == pNode || ! pNode->Is<NodeChar>())
                    {
                        return signalError(Regex::Error_InvalidBackslash);
                    }

                    char16 const wchMax = pNode->StaticCast<NodeChar>()->
                        GetChar();

                    if (wchMin > wchMax) {
                        return signalError(Regex::Error_InvalidRange);
                    }

                    pCharClass->Append(new(m_pHeap) NodeRange(
                        getDir(),
                        wchMin,
                        wchMax,
                        getCase() ) );

                    eState = State_Min;
                    break;
                } // Backslash

                default:
                    if (wchMin == wch)
                    {
                        pCharClass->Append(newChar(wchMin));
                    }
                    else if (wchMin < wch)
                    {
                        pCharClass->Append(
                            new(m_pHeap) NodeRange(
                                getDir(),
                                wchMin,
                                wch,
                                getCase() ) );
                    }
                    else
                    {
                        return signalError(Regex::Error_InvalidRange);
                    }

                    eState = State_Min;
                    break;
                } // switch
                break;

            case State_Min:
                switch (wch)
                {
                case '-':
                    return signalError(Regex::Error_InvalidRange);

                case Backslash:
                    goto handleMinBackslash;

                case CloseBracket:
                    return pCharClass->Simplify(m_pIEnv, m_pHeap);

                default:
                    wchMin = wch;
                    eState = State_Dash;
                    break;
                } // switch
                break;

            handleMinBackslash:
            {
                Node* const pNode = getTokenBackslashAux(
                    BackslashFlavor_RangeMin );

                if (NULL == pNode) {
                    return signalError(Regex::Error_InvalidBackslash);
                }

                if (pNode->Is<NodeChar>())
                {
                    if (State_Dash == eState)
                    {
                        pCharClass->Append(newChar(wchMin));
                    }
                    wchMin = pNode->StaticCast<NodeChar>()->GetChar();
                    eState = State_Dash;
                }
                else
                {
                    pCharClass->Append(pNode);
                    eState = State_Min;
                }
                break;
            } // handleMinBackslash

            default:
                NOTREACHED();
            } // switch state
        } // for
    } // parseCharClass

    private: Node* parseFactor(TokenType eEnd)
    {
        OpenScope oScope(this);

        Token oToken = getToken();
        switch (oToken.GetType())
        {
        case TokenType_Any:
            return new(m_pHeap) NodeAny(getDir());

        case TokenType_Atom:    // (?>...)
            return new(m_pHeap) NodeAtom(parse(TokenType_CloseParen));

        case TokenType_Capture: // (...)
        {
            if (isNoCapture())
            {
                return parse(TokenType_CloseParen);
            }
            m_cCaptures += 1;
            int iNth = m_cCaptures;
            Node* pNode = parse(TokenType_CloseParen);
            return new(m_pHeap) NodeCapture(getDir(), pNode, iNth);
        } // TokenType_Capture

        case TokenType_Group:   // (?:...)
            return parse(TokenType_CloseParen);

        case TokenType_If:
            return parseFactorIf();

        case TokenType_NamedCapture:
            return parseFactorNamedCapture();

        case TokenType_NegativeLookahead:
            return parseFactorLookahead(false);

        case TokenType_NegativeLookbehind:
            return parseFactorLookbehind(false);

        case TokenType_PositiveLookahead:
            return parseFactorLookahead(true);

        case TokenType_PositiveLookbehind:
            return parseFactorLookbehind(true);

        case TokenType_OptionOff:
        case TokenType_OptionOn:
            return parseFactorOption(eEnd, oToken.GetType());

        case TokenType_Or:
            ungetToken(oToken);
            return new(m_pHeap) NodeVoid;

        case TokenType_Asterisk:        // re*
        case TokenType_OpenBrace:       // re{...}
        case TokenType_Plus:            // re+
        case TokenType_Question:        // re?
            // quantifier apperas without re.
            return syntaxError();

        case TokenType_MultiLineDollar:
            if (isBackward())
            {
                return new(m_pHeap) NodeOr(
                    newZeroWidth(Op_EndOfString),
                    new(m_pHeap) NodeLookaround(
                        new(m_pHeap) NodeChar(getReverse(), Newline),
                        true ) );
            }
            return newZeroWidth(Op_BeforeNewline);

        case TokenType_SingleLineDollar:
            if (isBackward())
            {
                return new(m_pHeap) NodeOr(
                    newZeroWidth(Op_EndOfString),
                    new(m_pHeap) NodeLookaround(
                        new(m_pHeap) NodeAnd(
                            new(m_pHeap) NodeChar(getReverse(), Newline),
                            newZeroWidth(Op_EndOfString) ),
                        true ) );
            }
            return newZeroWidth(Op_EndOfString);

        default:
            if (oToken.GetType() == eEnd)
            {
                ungetToken(oToken);
                return new(m_pHeap) NodeVoid;
            }

            ASSERT(NULL != oToken.m_pNode);
            return oToken.m_pNode;
        } // switch token
    } // parseFactor

    // (?(cond)then|no)
    private: Node* parseFactorIf()
    {
        Node* pCond = parseFactorIfCond();
        Node* pThen = parse(TokenType_CloseParen);
        NodeOr* pThenElse = pThen->DynamicCast<NodeOr>();
        if (NULL == pThenElse)
        {
            return new(m_pHeap) NodeIf(pCond, pThen, new(m_pHeap) NodeVoid);
        }

        Nodes* pNodes = pThenElse->GetNodes();
        Node* pFirst = pNodes->GetFirst();
        if (NULL != pFirst)
        {
            Node* pSecond = pFirst->GetNext();
            if (NULL != pSecond && NULL == pSecond->GetNext())
            {
                return new(m_pHeap) NodeIf(pCond, pFirst, pSecond);
            }
        }

        return syntaxError();
    } // parseFactorIf

    private: Node* parseFactorIfCond()
    {
        OpenScope oScope(this, m_pwch - 1);

        int iChar = getChar();

        ungetChar();    // a first name char
        ungetChar();    // open paren

        // (?(?...) ...)
        if ('?' == iChar)
        {
            Token oToken = getToken();
            switch (oToken.GetType())
            {
            case TokenType_NegativeLookahead:
                return parseFactorLookahead(false);

            case TokenType_NegativeLookbehind:
                return parseFactorLookbehind(false);

            case TokenType_PositiveLookahead:
                return parseFactorLookahead(true);

            case TokenType_PositiveLookbehind:
                return parseFactorLookbehind(true);

            default:
                return syntaxError();
            } // switch
        } // if '?'

        // (?(name)...)
        CharSink oName(m_pHeap);
        int iNth = getNamedCapture(&oName);

        if (iNth < 0) return new(m_pHeap) NodeVoid;

        if (0 == iNth)
        {
            CaptureDef* pCaptureDef = findNamedCapture(oName.GetStart());

            if (NULL != pCaptureDef)
            {
                return new(m_pHeap) NodeCaptureIfNot(
                    getDir(),
                    pCaptureDef->m_iNth );
            }
        } // if

        NodeCaptureIfNot* pNode = new(m_pHeap) NodeCaptureIfNot(
            getDir(),
            iNth );

        if (0 == iNth || iNth > m_cCaptures)
        {
            CaptureRef* pCaptureRef = new(m_pHeap) CaptureRef(
                pNode,
                getSourcePosn(),
                oName.Save(m_pHeap) );

            m_oCaptureRefs.Append(pCaptureRef);
        } // if

        return pNode;
    } // parseFactorIfCond

    // (?= ...)
    // (?! ...)
    private: Node* parseFactorLookahead(bool fPositive)
    {
        Node* pNode;
        if (isBackward())
        {
            pNode = parse(TokenType_CloseParen);
            pNode->Reverse();
        }
        else
        {
            uint rgfOption = m_rgfOption;
            m_rgfOption &= ~Regex::Option_Backward;
            pNode = parse(TokenType_CloseParen);
            m_rgfOption = rgfOption;
        }

        return new(m_pHeap) NodeLookaround(pNode, fPositive);
    } // parseFactorLookahead

    // (?<= ...)
    // (?<! ...)
    private: Node* parseFactorLookbehind(bool fPositive)
    {
        Node* pNode;
        if (isBackward())
        {
            uint rgfOption = m_rgfOption;
            m_rgfOption &= ~Regex::Option_Backward;
            pNode = parse(TokenType_CloseParen);
            m_rgfOption = rgfOption;
        }
        else
        {
            uint rgfOption = m_rgfOption;
            m_rgfOption |= Regex::Option_Backward;
            pNode = parse(TokenType_CloseParen);
            m_rgfOption = rgfOption;
            pNode->Reverse();
        }

        return new(m_pHeap) NodeLookaround(pNode, fPositive);
    } // parseFactorLookahead

    private: Node* parseFactorNamedCapture()
    {
        CharSink oName(m_pHeap);
        int iNth = getNamedCapture(&oName);

        if (iNth < 0) return new(m_pHeap) NodeVoid;

        if (0 == iNth)
        {
            m_cCaptures += 1;

            iNth = m_cCaptures;

            CaptureDef* pCaptureDef = new(m_pHeap) CaptureDef(
                oName.Save(m_pHeap),
                iNth );

            m_pTree->m_oCaptures.Append(pCaptureDef);
        }
        else if (iNth == m_cCaptures + 1)
        {
            m_cCaptures += 1;
        }
        else if (iNth > m_cCaptures)
        {
            return signalError(Regex::Error_InvalidName);
        }

        Node* pNode = parse(TokenType_CloseParen);
        return new(m_pHeap) NodeCapture(getDir(), pNode, iNth);
    } // parseFactorNamedCapture

    private: Node* parseFactorOption(TokenType eEof, TokenType eState)
    {
        int rgfOn  = 0;
        int rgfOff = 0;
        for (;;)
        {
            int iChar = getChar();
            if (Eof == iChar) return unexpectedEof();
            switch (iChar)
            {
            #define case_ch_option(ch, opt) \
                case ch: rgfOff |= Regex::Option_ ## opt; break;

            case_ch_option('i', IgnoreCase)
            case_ch_option('m', Multiline)
            case_ch_option('n', ExplicitCapture)
            case_ch_option('s', Singleline)
            case_ch_option('u', Unicode)
            case_ch_option('x', ExtendedSyntax)

            case CloseParen:
                switch (eState)
                {
                case TokenType_OptionOff:
                    m_rgfOption &= ~rgfOff;
                    break;

                case TokenType_OptionOn:
                    rgfOn  = rgfOff;
                    break;
                default:
                    NOTREACHED();
                } // switch state

                m_rgfOption |= rgfOn;
                return parseFactor(eEof);

            case '-':
                switch (eState)
                {
                case TokenType_OptionOff:
                    return signalError(Regex::Error_Syntax);

                case TokenType_OptionOn:
                    rgfOn  = rgfOff;
                    rgfOff = 0;
                    eState = TokenType_OptionOff;
                    break;

                default:
                    NOTREACHED();
                } // switch state
                break;

            case ':':   // (?mod: ...)
            {
                int rgfSave = m_rgfOption;

                switch (eState)
                {
                case TokenType_OptionOff:
                    m_rgfOption &= ~rgfOff;
                    break;

                case TokenType_OptionOn:
                    rgfOn  = rgfOff;
                    break;
                default:
                    NOTREACHED();
                } // switch state

                m_rgfOption |= rgfOn;
                Node* pNode = parse(TokenType_CloseParen);
                m_rgfOption = rgfSave;
                return pNode;
            } // ':'

            default:
                return signalError(Regex::Error_Syntax);
            } // switch char
        } // for
    } // parseFactorOption

    //////////////////////////////////////////////////////////////////////
    //
    // parseQnt
    //
    // parse-qnt
    //  qnt ::= repetable '*'           greedy
    //      |   repetable '+'
    //      |   repetable '?'
    //      |   repetable '*?'          lazy
    //      |   repetable '+?'
    //      |   repetable '??'
    //      |   repetable '*+'          possessive
    //      |   repetable '++'
    //      |   repetable '?+'
    //      |   repetable '{' digit+ [',' [digit+]] '}' ('?' | '+')?
    //
    private: Node* parseQnt(TokenType eEnd)
    {
        Node* pNode = parseFactor(eEnd);

        for (;;)
        {
            Token oToken = getToken();

            MinMax oMinMax;
            switch (oToken.GetType())
            {
            case TokenType_Asterisk:    // r* = r{0,}
                oMinMax.m_iMin = 0;
                oMinMax.m_iMax = Infinity;
                break;

            case TokenType_Plus:        // r+ = r{1,}
                oMinMax.m_iMin = 1;
                oMinMax.m_iMax = Infinity;
                break;

            case TokenType_Question:    // r? = r{0,1}
                oMinMax.m_iMin = 0;
                oMinMax.m_iMax = 1;
                break;

            case TokenType_OpenBrace:   // r{m,n}
            {
                oMinMax = parseQntBrace();
                if (oMinMax.m_iMin < 0)
                    { return signalError(Regex::Error_InvalidBrace); }
                break;
            } // Token_LBrace

            default:
                ungetToken(oToken);
                return pNode;
            } // switch token

            // Note: It is nice to singal error when subexpr is zero-width
            // assertion, such as \b, (?<=expr). But, Perl and .NET allows it.
            if (oMinMax.m_iMin > oMinMax.m_iMax)
            {
                return signalError(Regex::Error_InvalidMinMax);
            }

            // Note: We should not  allow zero-width repetation even if Perl
            // and .NET accept them.
            #if 0
            if (0 == pNode->ComputeMaxLength())
            {
                return signalError(Regex::Error_InvalidQuantifier);
            }
            #endif

            bool fPossessive = false;
            switch (getChar())
            {
            case '?':
                unless (oMinMax.IsOne())
                {
                    pNode = new(m_pHeap) NodeMin(getDir(), pNode, oMinMax);
                }
                break;

            case '+':
                unless (oMinMax.IsOne())
                {
                    pNode = new(m_pHeap) NodeMax(getDir(), pNode, oMinMax);
                }
                fPossessive = true;
                break;

            case '*':
                return syntaxError();

            default:
                ungetChar();
                unless (oMinMax.IsOne())
                {
                    pNode = new(m_pHeap) NodeMax(getDir(), pNode, oMinMax);
                }
                break;
            } // switch

            pNode->Simplify(m_pIEnv, m_pHeap);

            if (fPossessive)
            {
                pNode = new(m_pHeap) NodeAtom(pNode);
                pNode->Simplify(m_pIEnv, m_pHeap);
            }
        } // for
    } // parseQnt

    // parse-qnt/brace -- '{' digit+ [ ',' [ digit+ ]] '}'
    //  Parses brace syntax and returns min and max.
    private: MinMax parseQntBrace()
    {
        MinMax oMinMax;
        oMinMax.m_iMin = getUInt();
        switch (getChar())
        {
        case CloseBrace:
            // r{n}
            oMinMax.m_iMax = oMinMax.m_iMin;
            return oMinMax;

        case ',':
            if (CloseBrace == getChar())
            {
                // r{n,}
                oMinMax.m_iMax = Infinity;
                return oMinMax;
            }
            ungetChar();
            oMinMax.m_iMax = getUInt();
            if (CloseBrace == getChar())
            {
                // r{n,m}
                return oMinMax;
            }
            break;
        } // switch char

        return MinMax();
    } // parseQntBase

    // [R]
    public: Tree* Run()
    {
        m_pTree = new(m_pHeap) Tree(m_rgfOption);
        if (NULL == m_pTree)
        {
            return nullptr;
        }

        if (m_rgfOption & Regex::Option_ExactString)
        {
            Node* pNode = new(m_pHeap) NodeString(
                getDir(),
                m_pwchStart,
                static_cast<int>(m_pwchEnd - m_pwchStart),
                getCase() );

            if (m_rgfOption & Regex::Option_ExactWord)
            {
                pNode = new(m_pHeap) NodeAnd(
                    newZeroWidth(isUnicode() ?
                        Op_UnicodeBoundary :
                        Op_AsciiBoundary ),
                    pNode,
                    newZeroWidth(isUnicode() ?
                        Op_UnicodeBoundary :
                        Op_AsciiBoundary ) );
            } // if word

            m_pTree->m_pNode = pNode;
        }
        else
        {
            parseAll();

            if (isBackward())
            {
                m_pTree->m_pNode->Reverse();
            }
        } // if

        return m_pTree;
    } // run

    // [S]
    private: Node* signalError(Regex::Error eError)
    {
        #if _DEBUG
            printf("Parse error %d at %d\n",
                eError,
                getSourcePosn() );
        #endif // _DEBUG

        if (0 == m_pTree->m_iErrorCode)
        {
            m_pTree->m_iErrorCode = eError;
            m_pTree->m_lErrorPosn = getSourcePosn();
        }

        return new(m_pHeap) NodeVoid;
    } // signalError

    private: Node* syntaxError()
    {
        return signalError(Regex::Error_Syntax);
    } // syntaxError

    // [U]
    private: Node* unclosedPair(Char)
    {
        const char16* const pwch = m_pwch;
        if (NULL != m_pwchOpen)
        {
            m_pwch = m_pwchOpen;
        }

        Node* const pNode = signalError(Regex::Error_UnclosedPair);
        m_pwch = pwch;
        return pNode;
    } // unclosedPair

    private: Node* unexpectedEof()
    {
        return signalError(Regex::Error_Eof);
    } // unexpectedEof

    private: void ungetChar()
    {
        ASSERT(m_pwch > m_pwchStart);
        --m_pwch;
    } // ugnetChar

    private: void ungetToken(Token oToken)
    {
        ASSERT(TokenType_None == m_oUngetToken.GetType());
        m_oUngetToken = oToken;
    } // ungetToken
}; // Parser

//////////////////////////////////////////////////////////////////////
//
// ParseRegex
//
Tree* ParseRegex(
    IEnvironment*   pIEnv,
    LocalHeap*      pHeap,
    const char16*   pwch,
    int             cwch,
    int             rgfOption )
{
    Parser oParser(pIEnv, pHeap, pwch, cwch, rgfOption);
    return oParser.Run();
} // ParseRegex

} // RegexPrivate
} // Regex
