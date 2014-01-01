#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Exact String Match
// listener/winapp/RegexMatcher.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/RegexMatch.cpp#6 $
//
#include "./RegexMatch.h"

#include "evita/core/buffer.h"
#include "evita/dom/range.h"

namespace Private
{

/// <remark>
///   Represents regex capture.
/// </remark>
class Capture
{
    public: Edit::Range*    m_pRange;
    public: char16*         m_pwszName;

    public: Capture() :
        m_pRange(NULL),
        m_pwszName(NULL) {}
}; // Capture

typedef unsigned char uint8;

/// <remark>
///   Represents regex compilation context.
/// </remark>
class CompileContext : public Regex::ICompileContext
{
    public: int        m_cCaptures;
    public: int        m_nError;
    public: int        m_nPosn;
    public: uint8*     m_prgbBlob;
    public: Capture*   m_prgoCapture;

    public: CompileContext() :
        m_cCaptures(0),
        m_nError(0),
        m_nPosn(0),
        m_prgbBlob(NULL),
        m_prgoCapture(NULL) {}

    public: virtual ~CompileContext()
    {
        delete m_prgbBlob;
        delete m_prgoCapture;
    } // ~CompileContext

    private: virtual void* AllocRegex(size_t cb, int cCaptures) override
    {
        ASSERT(0 == m_cCaptures);

        ASSERT(cb >= 1);
        ASSERT(cCaptures >= 0);

        m_cCaptures   = cCaptures;
        m_prgbBlob    = new uint8[cb];
        m_prgoCapture = new Capture[static_cast<size_t>(cCaptures + 1)];

        return m_prgbBlob;
    } // Alloc

    private: virtual bool SetCapture(int iNth, const char16* pwsz) override
    {
        int cwch = ::lstrlenW(pwsz);
        char16* pwszSave = new char16[static_cast<size_t>(cwch + 1)];
        ::lstrcpy(pwszSave, pwsz);
        m_prgoCapture[iNth].m_pwszName = pwszSave;
        return true;
    } // SetCapture

    private: virtual void SetError(int nPosn, int nError) override
    {
        m_nError = nError;
        m_nPosn  = nPosn;
    } // SetError
}; // CompileContext

/// <remark>
///   Represents match context regex again Buffer.
/// </remark>
/// <see cref="Edit::Buffer"/>
class BufferMatchContext : public Regex::IMatchContext
{
    private: int                m_cCaptures;
    private: Edit::Range        m_oRange;
    private: CompileContext*    m_pCompileContext;
    private: Regex::IRegex*     m_pIRegex;
    private: Capture*           m_prgoCapture;

    public: BufferMatchContext(
        Regex::IRegex*  pIRegex,
        CompileContext* pCompileContext,
        Capture*        prgoCapture,
        int             cCaptures,
        Edit::Buffer*   pBuffer,
        Posn            lStart,
        Posn            lEnd ) :
            m_cCaptures(cCaptures),
            m_oRange(pBuffer, lStart, lEnd),
            m_pCompileContext(pCompileContext),
            m_pIRegex(pIRegex),
            m_prgoCapture(prgoCapture)
    {
        ASSERT(NULL != pBuffer);
        ASSERT(NULL != m_pCompileContext);
        ASSERT(NULL != pIRegex);
        ASSERT(NULL != prgoCapture);
    } // BufferMatchContext

    public: virtual ~BufferMatchContext()
    {
        delete m_pCompileContext;
    } // ~BufferMatchContext

    // [C]
    private: bool charEqCi(char16 wch1, char16 wch2) const
    {
        if (wch1 == wch2) return true;
        return ::CharUpcase(wch1) == ::CharUpcase(wch2);
    } // charEqCi

    private: static bool charEqCs(char16 wch1, char16 wch2)
    {
        return wch1 == wch2;
    } // charEqCi

    // [G]
    public: Edit::Range* GetCapture(int nNth) const
    { 
        return uint(nNth) <= uint(m_cCaptures)
            ? m_prgoCapture[nNth].m_pRange
            : nullptr;
    } // GetCapture

    public: Edit::Range* GetCapture(
        const char16*   pwchName,
        int             cwchName ) const
    {
        for (int nNth = 1; nNth <= m_cCaptures; nNth++)
        {
            const char16* pwch = pwchName;
            const char16* pwchEnd = pwchName + cwchName;
            const char16* pwsz = m_prgoCapture[nNth].m_pwszName;
            while (0 != *pwsz)
            {
                if (pwch == pwchEnd)
                {
                    return false;
                }

                if (*pwsz == *pwch)
                {
                    // ok
                }
                else if (CharUpcase(*pwsz) == CharUpcase(*pwch))
                {
                    // ok
                }
                else
                {
                    return false;
                }
            } // while

            if (0 == *pwsz)
            {
                return m_prgoCapture[nNth].m_pRange;
            }
        } // for nNth

        return NULL;
    } // GetCapture

    public: Edit::Range* GetRange() const
        { return const_cast<Edit::Range*>(&m_oRange); }

    // Regex::IMatchContext
    // [B]
    virtual bool BackwardFindCharCi(char16, Posn*, Posn) const override;
    virtual bool BackwardFindCharCs(char16, Posn*, Posn) const override;

    // [F]
    virtual bool ForwardFindCharCi(char16, Posn*, Posn) const override;
    virtual bool ForwardFindCharCs(char16, Posn*, Posn) const override;

    // [G]
    virtual bool GetCapture(int, Posn*, Posn*) const override;

    virtual char16 GetChar(Posn lPosn) const override
        { return m_oRange.GetBuffer()->GetCharAt(lPosn); }

    virtual Posn GetEnd() const override
        { return m_oRange.GetEnd(); }

    virtual void GetInfo(Regex::SourceInfo* p) const override
    {
        p->m_lStart = 0;
        p->m_lEnd   = m_oRange.GetBuffer()->GetEnd();

        p->m_lScanStart = m_oRange.GetStart();
        p->m_lScanEnd   = m_oRange.GetEnd();
    } // GetInfo

    virtual Posn GetStart() const override
        { return m_oRange.GetStart(); }

    // [R]
    virtual void ResetCapture(int) override;
    virtual void ResetCaptures() override;

    // [S]
    virtual void SetCapture(int, Posn, Posn) override;
    virtual bool StringEqCi(const char16*, int, Posn) const override;
    virtual bool StringEqCs(const char16*, int, Posn) const override;
}; // BufferMatchContext

// [B]
bool BufferMatchContext::BackwardFindCharCi(
    char16  wchFind,
    Posn*   inout_lPosn,
    Posn    lStop ) const
{
    Edit::Buffer::EnumCharRev::Arg oArg(
        m_oRange.GetBuffer(),
        *inout_lPosn,
        lStop );

    foreach (Edit::Buffer::EnumCharRev, oEnum, oArg)
    {
        if (charEqCi(oEnum.Get(), wchFind))
        {
            *inout_lPosn = oEnum.GetPosn();
            return true;
        }
    } // while
    return false;
} // BufferMatchContext::BackwardFindCharCi

bool BufferMatchContext::BackwardFindCharCs(
    char16  wchFind,
    Posn*   inout_lPosn,
    Posn    lStop ) const
{
    Edit::Buffer::EnumCharRev::Arg oArg(
        m_oRange.GetBuffer(),
        *inout_lPosn,
        lStop );

    foreach (Edit::Buffer::EnumCharRev, oEnum, oArg)
    {
        if (charEqCs(oEnum.Get(), wchFind))
        {
            *inout_lPosn = oEnum.GetPosn();
            return true;
        }
    } // while
    return false;
} // BufferMatchContext::BackwardFindCharCs

bool BufferMatchContext::ForwardFindCharCi(
    char16  wchFind,
    Posn*   inout_lPosn,
    Posn    lStop ) const
{
    Edit::Buffer::EnumChar::Arg oArg(
        m_oRange.GetBuffer(),
        *inout_lPosn,
        lStop );

    foreach (Edit::Buffer::EnumChar, oEnum, oArg)
    {
        if (charEqCi(oEnum.Get(), wchFind))
        {
            *inout_lPosn = oEnum.GetPosn();
            return true;
        }
    } // while
    return false;
} // BufferMatchContext::ForwardFindCharCi

// [F]
bool BufferMatchContext::ForwardFindCharCs(
    char16  wchFind,
    Posn*   inout_lPosn,
    Posn    lStop ) const
{
    Edit::Buffer::EnumChar::Arg oArg(
        m_oRange.GetBuffer(),
        *inout_lPosn,
        lStop );

    foreach (Edit::Buffer::EnumChar, oEnum, oArg)
    {
        if (charEqCs(oEnum.Get(), wchFind))
        {
            *inout_lPosn = oEnum.GetPosn();
            return true;
        }
    } // while
    return false;
} // BufferMatchContext::ForwardFindCharCs

// [G]
bool BufferMatchContext::GetCapture(
    int             nCapture,
    Regex::Posn*    out_lStart,
    Regex::Posn*    out_lEnd ) const
{
    Edit::Range* pRange = m_prgoCapture[nCapture].m_pRange;
    if (NULL == pRange)
    {
        return false;
    }

    *out_lStart = pRange->GetStart();
    *out_lEnd   = pRange->GetEnd();

    return true;
} // BufferMatchContext::GetCapture

// [R]
void BufferMatchContext::ResetCapture(int nCapture)
{
    if (Edit::Range* pRange = m_prgoCapture[nCapture].m_pRange)
    {
        pRange->destroy();
        m_prgoCapture[nCapture].m_pRange = NULL;
    }
} // BufferMatchContext::ResetCapture

void BufferMatchContext::ResetCaptures()
{
    for (int nNth = 0; nNth < m_cCaptures; nNth++)
    {
        ResetCapture(nNth);
    } // for nNth
} // BufferMatchContext::ResetCaptures

// [S]
void BufferMatchContext::SetCapture(int nCapture, Posn lStart, Posn lEnd)
{
    ASSERT(static_cast<uint>(nCapture) <= static_cast<uint>(m_cCaptures));

    Edit::Range* pRange = m_prgoCapture[nCapture].m_pRange;
    if (NULL == pRange)
    {
        Edit::Buffer* pBuffer = m_oRange.GetBuffer();
        pRange = new(pBuffer->GetHeap()) Edit::Range(pBuffer, lStart, lEnd);
        m_prgoCapture[nCapture].m_pRange = pRange;
    }
    else
    {
        pRange->SetRange(lStart, lEnd);
    }
} // BufferMatchContext::SetCapture

bool BufferMatchContext::StringEqCi(
    const char16*   pwchStart,
    int             cwch,
    Posn            lPosn ) const
{
    Edit::Buffer::EnumChar::Arg oArg(
        m_oRange.GetBuffer(),
        lPosn );

    Edit::Buffer::EnumChar oEnum(oArg);
    const char16* pwchEnd = pwchStart + cwch;
    for (const char16* pwch = pwchStart; pwch < pwchEnd; pwch++)
    {
        if (oEnum.AtEnd())
        {
            return false;
        }

        if (! charEqCi(*pwch, oEnum.Get()))
        {
            return false;
        }

        oEnum.Next();
    } // for pwch
    return true;
} // BufferMatchContext::StringEqCi

bool BufferMatchContext::StringEqCs(
    const char16*   pwchStart,
    int             cwch,
    Posn            lPosn ) const
{
    Edit::Buffer::EnumChar::Arg oArg(
        m_oRange.GetBuffer(),
        lPosn );

    Edit::Buffer::EnumChar oEnum(oArg);
    const char16* pwchEnd = pwchStart + cwch;
    for (const char16* pwch = pwchStart; pwch < pwchEnd; pwch++)
    {
        if (oEnum.AtEnd())
        {
            return false;
        }

        if (! charEqCs(*pwch, oEnum.Get()))
        {
            return false;
        }

        oEnum.Next();
    } // for pwch
    return true;
} // BufferMatchContext::StringEqCs

} // private

using namespace Private;

// ctor
/// <summary>Construct RegexMatcher object</summary>
/// <param name="pSearch">Matching settings</param>
/// <param name="pBuffer">A Edit::Buffer to match</param>
/// <param name="lStart">Match start position</param>
/// <param name="lEnd">Match end position (exclusive)</param>
RegexMatcher::RegexMatcher(
    const SearchParameters* pSearch,
    Edit::Buffer*           pBuffer,
    Posn                    lStart,
    Posn                    lEnd ) :
        m_fMatched(false),
        m_oSearch(*pSearch),
        m_pMatchContext(NULL)
{
    CompileContext* pCompileContext = new CompileContext;

    uint rgfFlag = Regex::Option_ExactString;

    if (pSearch->m_rgf & SearchFlag_Backward)
    {
        rgfFlag |= Regex::Option_Backward;
    }

    if (pSearch->m_rgf & SearchFlag_IgnoreCase)
    {
        rgfFlag |= Regex::Option_IgnoreCase;
    }

    // FIXME 2008-07-08 yosi@msn.com How do we handle SearchFlag_MatchWord?
    if (pSearch->m_rgf & SearchFlag_Regex)
    {
        rgfFlag &= ~Regex::Option_ExactString;
        rgfFlag |= Regex::Option_Multiline;
        rgfFlag |= Regex::Option_Unicode;
    }
    else if (pSearch->m_rgf & SearchFlag_MatchWord)
    {
        rgfFlag |= Regex::Option_ExactWord;
    }

    // FIXME 2008-07-08 yosi@msn.com We should not allocate Regex object
    // in Regex::Compiler method.
    m_pIRegex = Regex::Compile(
        pCompileContext,
        m_oSearch.m_wsz,
        ::lstrlenW(m_oSearch.m_wsz),
        static_cast<int>(rgfFlag));

    if (NULL == m_pIRegex)
    {
        m_oErrorInfo.m_nError = pCompileContext->m_nError;
        m_oErrorInfo.m_nPosn  = pCompileContext->m_nPosn;
        return;
    }

    m_pMatchContext = new BufferMatchContext(
        m_pIRegex,
        pCompileContext,
        pCompileContext->m_prgoCapture,
        pCompileContext->m_cCaptures,
        pBuffer,
        lStart,
        lEnd );
} // RegexMatcher::RegexMatcher

// [F]

/// <summary>
///   Finds the first match.
/// </summary>
/// <returns>
///   True if there is match, false otherwise.
/// </returns>
bool RegexMatcher::FirstMatch()
{
    if (NULL == m_pIRegex)
    {
        return false;
    }

    if (NULL == m_pMatchContext)
    {
        return false;
    }

    return m_fMatched = Regex::StartMatch(m_pIRegex, m_pMatchContext);
} // RegexMatcher::FirstMatch

// [G]

/// <summary>
///   Retreives captured range of specified position. Position 0 specifies
///   entire matched text. Position 1 specifies the first captur, 2 is
///   second capture and so on.
///   <para>
///     This method returns NULL in following cases:
///     <list>
///       <item><description>
///         The last match is failed.
///       </description></item>
///       <item><description>
///         Specified number of capture doesn't exist.
///       </description></item>
///     </list>
///   </para>
/// </summary>
/// <param name="pwchName">A capture name</param>
/// <param name="cwchName">A length of capture name</param>
/// <returns>A Edit::Range of nth capture.</returns>
Edit::Range* RegexMatcher::GetMatched(int nNth)
{
    if (! m_fMatched) 
    {
        return NULL;
    }

    if (NULL == m_pIRegex)
    {
        return false;
    }

    if (NULL == m_pMatchContext)
    {
        return false;
    }

    return m_pMatchContext->GetCapture(nNth);
} // RegexMatcher::GetMatch

/// <summary>
///   Retreives captured range of specified name.
/// </summary>
/// <param name="pwchName">A capture name</param>
/// <param name="cwchName">A length of capture name</param>
Edit::Range* RegexMatcher::GetMatched(const char16* pwchName, int cwchName)
{
    if (! m_fMatched) 
    {
        return NULL;
    }

    if (NULL == m_pIRegex)
    {
        return false;
    }

    if (NULL == m_pMatchContext)
    {
        return false;
    }

    return m_pMatchContext->GetCapture(pwchName, cwchName);
} // RegexMatcher::GetMatch

// [N]

/// <summary>
///   Finds next match.
/// </summary>
bool RegexMatcher::NextMatch()
{
    if (! m_fMatched)
    {
        return false;
    }

    if (NULL == m_pMatchContext)
    {
        return false;
    }

    return m_fMatched = Regex::NextMatch(m_pIRegex, m_pMatchContext);
} // RegexMatcher::NextMatch

// [R]

class EnumChar
{
    private: const char16*   m_pwch;
    private: const char16*   m_pwchEnd;

    public: struct Arg
    {
        int             m_cwch;
        const char16*   m_pwch;

        Arg(const char16* pwch, int cwch) :
            m_cwch(cwch), m_pwch(pwch) {}
    }; // Arg

    public: EnumChar(Arg oArg) :
        m_pwch(oArg.m_pwch),
        m_pwchEnd(oArg.m_pwch + oArg.m_cwch) {}

    public: bool   AtEnd() const { return m_pwch >= m_pwchEnd; }
    public: char16 Get()   const { ASSERT(!AtEnd()); return *m_pwch; }
    public: void   Next()        { ASSERT(!AtEnd()); m_pwch++; }
}; // EnumChar

/// <summary>
///   Inserts pwchWith after matched text then delete matched text and
///   set matched text as replaced text.
/// </summary>
class BufferOutputStream
{
    int             m_cwch;
    int             m_cwchTotal;
    Edit::Range     m_oRange;
    Edit::Range*    m_pMatchedRange;
    char16          m_rgwch[80];

    public: BufferOutputStream(
        Edit::Range*     pMatchedRange ) :
        m_cwch(0),
        m_cwchTotal(0),
        m_oRange(pMatchedRange),
        m_pMatchedRange(pMatchedRange)
    {
        m_oRange.Collapse(Collapse_End);
    } // BufferOutputStream

    public: ~BufferOutputStream()
    {
        flush();

        // Delete matched string
        m_pMatchedRange->SetText(NULL, 0);
        m_pMatchedRange->MoveEnd(Unit_Char, m_cwchTotal);
    } // ~BufferOutputStream

    // [G]
    private: void flush()
    { 
        m_cwchTotal += m_cwch;
        m_oRange.SetText(m_rgwch, m_cwch);
        m_oRange.Collapse(Collapse_End);
        m_cwch = 0;
    } // Flush

    // [W]
    public: void Write(char16 wch)
    {
        ASSERT(m_cwch <= lengthof(m_rgwch));

        if (m_cwch == lengthof(m_rgwch))
        {
            flush();
            ASSERT(0 == m_cwch);
        }

        m_rgwch[m_cwch] = wch;
        m_cwch += 1;
    } // Write

    public: void Write(const char16* pwch, int cwch)
    {
        foreach (EnumChar, oEnum, EnumChar::Arg(pwch, cwch))
        {
            Write(oEnum.Get());
        } // for pwch
    } // Write

    public: void Write(Edit::Range* pRange)
    {
        foreach (Edit::Buffer::EnumChar, oEnum, pRange)
        {
            Write(oEnum.Get());
        } // for
    } // Write
}; // BufferOutputStream

/// <summary>
///   Replaces matched test with specified replacement string.
///   Meta character in replacement string:
///   <list>
///     <item>
///       <term><c>$0, $1, ..., $n</c></term>
///       <description>Replace with captured string</description>
///     </item>
///     <item>
///       <term><c>$&amp;</c></term>
///       <description>Replace with the last matched string</description>
///     </item>
///     <item>
///       <term><c>${</c><i>name</i><c>}</c></term>
///       <description>Replace with named captuer</description>
///     </item>
///     <item>
///       <term><c>\</c><i>C</i></term>
///       <description>Escape next character</description>
///     </item>
///     <item>
///       <term><c>\a</c></term>
///       <description>Replace with U+0007</description>
///     </item>
///     <item>
///       <term><c>\b</c></term>
///       <description>Replace with U+0008</description>
///     </item>
///     <item>
///       <term><c>\c</c><i>C</i></term>
///       <description>Replace with U+0007</description>
///     </item>
///     <item>
///       <term><c>\e</c></term>
///       <description>Replace with U+001B</description>
///     </item>
///     <item>
///       <term><c>\f</c></term>
///       <description>Replace with U+000C</description>
///     </item>
///     <item>
///       <term><c>\n</c></term>
///       <description>Replace with U+000A</description>
///     </item>
///     <item>
///       <term><c>\r</c></term>
///       <description>Replace with U+000D</description>
///     </item>
///     <item>
///       <term><c>\t</c></term>
///       <description>Replace with U+0009</description>
///     </item>
///     <item>
///       <term><c>\u</c><i>UUUU</i></term>
///       <description>Replace with Unicode character</description>
///     </item>
///     <item>
///       <term><c>\x</c><i>XX</i></term>
///       <description>Replace with Unicode character</description>
///     </item>
///     <item>
///       <term><c>\x{</c><i>XXXX</i><c>}</c></term>
///       <description>Replace with Unicode character</description>
///     </item>
///   </list>
/// </summary>
/// <param name="pwchWith">Replacement string</param>
/// <param name="cwchWith">Number of characters in replacement string</param>
/// <param name="fMetaChar">True if enabling meta character</param>
void RegexMatcher::Replace(
    const char16*   pwchWith,
    int             cwchWith,
    bool            fMetaChar )
{
    Edit::Range* pRange = GetMatched(0);
    if (NULL == pRange)
    {
        // This matcher isn't matched.
        return;
    }

    if (! fMetaChar)
    {
        pRange->SetText(pwchWith, cwchWith);
        return;
    }

    class Util
    {
        public: static bool IsDigit(char16 wch, int* out_iDigit)
        {
            if (wch >= '0' && wch <= '9')
            {
                *out_iDigit = wch - '0';
                return true;
            }
            return false;
        } // IsDigit

        public: static bool IsNameFirst(char16 wch)
        {
            if (wch >= 'A' && wch <= 'Z') return true;
            if (wch >= 'a' && wch <= 'a') return true;
            if ('.' == wch) return true;
            if ('-' == wch) return true;
            if ('_' == wch) return true;
            return false;
        } // IsNameFirst

        public: static bool IsXDigit(char16 wch, int* out_iDigit)
        {
            if (wch >= '0' && wch <= '9')
            {
                *out_iDigit = wch - '0';
            }
            else if (wch >= 'A' && wch <=' F')
            {
                *out_iDigit = wch - 'A' + 10;
            }
            else if (wch >= 'a' && wch <=' f')
            {
                *out_iDigit = wch - 'a' + 10;
            }
            else
            {
                return false;
            }
            return true;
        } // IsXDigit
    }; // Util

    enum State
    {
        State_Normal,

        State_Capture,      // \<digit>+
        State_Dollar,
        State_Escape,
        State_Escape_0,     // \0oo
        State_Escape_c,     // \cC
        State_Escape_u,     // \uXXXX
        State_Escape_x,     // \xXX
        State_Escape_x4,    // \x{XXXX}
        State_Ref,          // ${name}
    } eState = State_Normal;

    int cChars = 0;
    int iAcc   = 0;

    BufferOutputStream oStream(pRange);

    const char16* pwchEnd = pwchWith + cwchWith;
    for (const char16* pwch = pwchWith; pwch < pwchEnd; pwch++)
    {
        char16 wch = *pwch;

      tryAgain:
        switch (eState)
        {
        case State_Normal:
            switch (wch)
            {
            case '$':
                if (cChars > 0)
                {
                    oStream.Write(pwch - cChars, cChars);
                    cChars = 0;
                }
                eState = State_Dollar;
                break;

            case '\\':
                if (cChars > 0)
                {
                    oStream.Write(pwch - cChars, cChars);
                    cChars = 0;
                }
                eState = State_Escape;
                break;

            default:
                cChars += 1;
                break;
            } // switch wch
            break;

        case State_Capture:
            if (wch >= '0' && wch <= '9')
            {
                iAcc *= 10;
                iAcc += wch - '0';
            }
            else
            {
                oStream.Write(GetMatched(iAcc));
                eState = State_Normal;
                goto tryAgain;
            }
            break;

        case State_Dollar:
            ASSERT(0 == cChars);
            switch (wch)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                iAcc = wch - '0';
                eState = State_Capture;
                break;

            case '&':
                oStream.Write(GetMatched(0));
                eState = State_Normal;
                break;

            case 0x7B:
                eState = State_Ref;
                break;

            default:
                // Following Perl notations are not special meaning.
                // $$
                // $+
                // $_ match-target
                // $` match-before
                // $' match-after
                cChars += 1;
                eState = State_Normal;
                break;
            } // switch wch
            break;

        case State_Escape:
            ASSERT(0 == cChars);
            eState = State_Normal;
            switch (wch)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7':
                cChars = 1;
                iAcc   = wch - '0';
                eState = State_Escape_0;
                break;

            case 'a':
                oStream.Write(0x07);
                break;

            case 'b':
                oStream.Write(0x08);
                break;

            case 'c':
                eState = State_Escape_c;
                break;

            case 'e':
                oStream.Write(0x1B);
                break;

            case 'f':
                oStream.Write(0x0C);
                break;

            case 'n':
                oStream.Write(0x0A);
                break;

            case 'r':
                oStream.Write(0x0D);
                break;

            case 't':
                oStream.Write(0x09);
                break;

            case 'u':
                cChars = 0;
                iAcc   = 0;
                eState = State_Escape_u;
                break;

            case 'v':
                oStream.Write(0x0B);
                break;

            case 'x':
                cChars = 0;
                iAcc   = 0;
                eState = State_Escape_u;
                break;

            case '\\':
                cChars = 1;
                break;

            default:
                // Insert escaped character instead of signal error
                cChars = 1;
                break;
            } // swtich wch
            break;

        case State_Escape_0:
            if (wch >= '0' && wch <= '7')
            {
                iAcc *= 8;
                iAcc |= wch - '0';
                cChars += 1;
            }
            else
            {
                --pwch;
                cChars = 3;
            }

            if (3 == cChars)
            {
                oStream.Write(static_cast<char16>(iAcc));
                cChars = 0;
                eState = State_Normal;
            }
            break;

        case State_Escape_c:
            if (wch >= '@' && wch <= 0x5F)
            {
                oStream.Write(static_cast<char16>(wch - '@'));
            }
            else if (wch >= 'a' && wch <= 'z')
            {
                oStream.Write(static_cast<char16>(wch - 0x60));
            }
            else
            {
                // ignore invalid replacement
            }
            eState = State_Normal;
            break;

        case State_Escape_u:
        {
            int iDigit;
            if (Util::IsXDigit(wch, &iDigit))
            {
                cChars += 1;
                iAcc *= 16;
                iAcc |= iDigit;

                if (4 == cChars)
                {
                    oStream.Write(static_cast<char16>(iAcc));
                    cChars = 0;
                }
            }
            else
            {
                // ignore invalid replacement
                eState = State_Normal;
            }
            break;
        } // State_Escape_u

        case State_Escape_x:
            if (Util::IsXDigit(wch, &iAcc))
            {
                iAcc = wch - '0';
                cChars = 3;
                eState = State_Escape_u;
            }
            else if (0x7B == wch)
            {
                ASSERT(0 == cChars);
                ASSERT(0 == iAcc);
                eState = State_Escape_x4;
            }
            else
            {
                // ignore invalid \xXX
                eState = State_Normal;
            }
            break;

        case State_Escape_x4:
        {
            int iDigit;
            if (4 == cChars)
            {
                if (0x7D == wch)
                {
                    oStream.Write(static_cast<char16>(iAcc));
                }

                cChars = 0;
                eState = State_Normal;
            }
            else if (Util::IsXDigit(wch, &iDigit))
            {
                iAcc *= 16;
                iAcc |= iDigit;
                cChars += 1;
            }
            else
            {
                // ignore invalid \uXXXX
                cChars = 0;
                eState = State_Normal;
            }
            break;
        } // State_Escape_x4

        case State_Ref:
            if (0x7D == wch)
            {
                if (Edit::Range* pRange = GetMatched(pwch - cChars, cChars))
                {
                    oStream.Write(pRange);
                }

                cChars = 0;
                eState = State_Normal;
            }
            else
            {
                cChars += 1;
            }
            break;

        default:
            CAN_NOT_HAPPEN();
        } // switch eState
    } // for pwch

    switch (eState)
    {
    case State_Normal:
        oStream.Write(pwchEnd - cChars, cChars);
        break;

    case State_Capture:
        if (auto const range = GetMatched(iAcc))
        {
            oStream.Write(GetMatched(iAcc));
        }
        break;
    } // switch eState
} // RegexMatcher::Replace

// [W]
/// <summary>
///   Continue to find match from start of document or end of document if
///   this match is applied for whole document.
/// </summary>
/// <returns>
///   True if there is matched text, otherwise returns false.
/// </returns>
bool RegexMatcher::WrapMatch()
{
    if (NULL == m_pIRegex)
    {
        return false;
    }

    if (NULL == m_pMatchContext)
    {
        return false;
    }

    if (! m_oSearch.IsWhole())
    {
        return false;
    }

    Edit::Range* pRange = m_pMatchContext->GetRange();

    if (m_oSearch.IsBackward())
    {
        pRange->SetEnd(pRange->GetBuffer()->GetEnd());
    }
    else
    {
        pRange->SetStart(pRange->GetBuffer()->GetStart());
    }

    return FirstMatch();
} // RegexMatcher::WraptMatch
