#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Plain Text Mode
// listener/winapp/mode_PlainText.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Mode.cpp#10 $
//
#include "./ed_Mode.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "./mode_Config.h"
#include "./mode_Cxx.h"
#include "./mode_Haskell.h"
#include "./mode_Lisp.h"
#include "./mode_Mason.h"
#include "./mode_Perl.h"
#include "./mode_PlainText.h"
#include "./mode_Python.h"
#include "./mode_Xml.h"

#include <commctrl.h>
#include <string>
#include <unordered_map>

#pragma comment(lib, "comctl32.lib")

namespace Edit
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

namespace {

//////////////////////////////////////////////////////////////////////
//
// LoadIconFromRegistry
//
// REVIEW 2007-08-05 yosi@msn.com Should we use SHGetFileInfo even if
// it requirres CoInitialize?
//
HICON LoadIconFromRegistry(const char16* pwszExt)
{
    long lError;

    RegKey hkeyExtension;
    lError = ::RegOpenKeyEx(
        HKEY_CLASSES_ROOT,
        pwszExt,
        0,
        KEY_QUERY_VALUE,
        &hkeyExtension.h );
    if (ERROR_SUCCESS != lError)
    {
        DEBUG_PRINTF("RegOpenKey HKCR/%ls %d\n", pwszExt, lError);
        return NULL;
    }

    char16 wszFile[1024];
    DWORD cbFile;
    lError = ::RegQueryValueEx(
        hkeyExtension,
        NULL,   // lpValueName
        NULL,   // lpReserved
        NULL,   // lpType
        reinterpret_cast<BYTE*>(wszFile),
        &cbFile );
    if (ERROR_SUCCESS != lError)
    {
        DEBUG_PRINTF("RegQueryValueEx HKCR/%ls %d\n", pwszExt, lError);
        return NULL;
    }

    ::lstrcatW(wszFile, L"\\DefaultIcon");

    RegKey hkeyFile;
    lError = ::RegOpenKeyEx(
        HKEY_CLASSES_ROOT,
        wszFile,
        0,
        KEY_QUERY_VALUE,
        &hkeyFile.h );
    if (ERROR_SUCCESS != lError)
    {
        DEBUG_PRINTF("RegOpenKey HKCR/%ls %d\n", wszFile, lError);
        return NULL;
    }

    char16 wszIcon[1024];
    DWORD cbIcon;
    lError = ::RegQueryValueEx(
        hkeyFile,
        NULL,   // lpValueName
        NULL,   // lpReserved
        NULL,   // lpType
        reinterpret_cast<BYTE*>(wszIcon),
        &cbIcon );
    if (ERROR_SUCCESS != lError)
    {
        DEBUG_PRINTF("RegQueryValueEx HKCR/%ls %d\n", pwszExt, lError);
        return NULL;
    }

    DEBUG_PRINTF("icon=%ls\n", wszIcon);

    char16* pwszComma = ::lstrchrW(wszIcon, ',');
    int iIndex = 0;
    if (NULL != pwszComma)
    {
        *pwszComma++ = 0;

        char16* pwszRunner = pwszComma;
        iIndex = 0;
        int iSign = 1;
        if ('-' == *pwszRunner)
        {
            pwszRunner++;
            iSign = -1;
        }

        while (0 != *pwszRunner)
        {
            iIndex *= 10;
            iIndex += *pwszRunner - '0';
            pwszRunner++;
        }

        iIndex *= iSign;
    } // if

    // Note: ExtractIconEx expands environment variables in
    // pathname, e.g. "%lsystemRoot%\system32\imageres.dll,-102".
    HICON hIcon;
    if (1 != ::ExtractIconEx(wszIcon, iIndex, NULL, &hIcon, 1))
    {
        return NULL;
    }

    return hIcon;
} // LoadIconFromRegistry

class IconCache : public common::Singleton<IconCache> {
  private: std::unordered_map<base::string16, int> map_;
  private: HIMAGELIST image_list_;

  public: IconCache()
      : image_list_(::ImageList_Create(16, 16, ILC_COLOR32, 10, 10)) {
  }

  public: ~IconCache() {
    if (image_list_)
      ::ImageList_Destroy(image_list_);
  }

  public: HIMAGELIST image_list() const { return image_list_; }

  public: void Add(const base::string16& name, int icon_index) {
    DCHECK_GE(icon_index, 0);
    map_[name] = icon_index;
  }

  public: int AddIcon(const base::string16& name, HICON icon) {
    auto const icon_index = ::ImageList_ReplaceIcon(image_list_, -1, icon);
    if (icon_index >= 0)
      Add(name, icon_index);
    return icon_index;
  }

  public: int Intern(const base::string16& name) {
    auto const it = map_.find(name);
    if (it != map_.end())
      return it->second + 1;

    if (auto const icon = LoadIconFromRegistry(name.c_str()))
      return AddIcon(name, icon) + 1;

    return 0;
  }
};

base::string16 GetExtension(const base::string16& name,
                             const base::string16& default_extension) {
  auto const index = name.find_last_of('.');
  return index == base::string16::npos ? default_extension :
                                         name.substr(index);
}

} // namespace

/// <summary>
///  Get icon handle from registry.
///  <list>
///    <item><term>HKCR/{extension}/@ = filekey</term></item>
///    <item><term>HKCR/{filekey}/DefaultIcon/@ = {path},{index}</term></item>
///  </list>
/// </summary>
int Mode::GetIcon() const {
  DEFINE_STATIC_LOCAL(const base::string16, default_ext, (L".txt"));

  const base::string16 ext = GetExtension(GetBuffer()->GetName(), default_ext);
  if (auto const icon_index = IconCache::instance().Intern(ext))
    return icon_index - 1;

  auto const default_icon_index = IconCache::instance().Intern(default_ext);
  ASSERT(default_icon_index);

  IconCache::instance().Add(ext, default_icon_index - 1);
  return default_icon_index - 1;
}

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

HIMAGELIST ModeFactory::icon_image_list() {
  return IconCache::instance().image_list();
}

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

        if (pModeFactory->IsSupported(pBuffer->GetFileName()))
        {
            return pModeFactory;
        }

        if (pModeFactory->IsSupported(pBuffer->GetName()))
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

} // Edit
