#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Commands and Bindings
// listener/winapp/cm_Commands.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/cm_commands.cpp#5 $
//
//
#include "evita/cm_CmdProc.h"

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/ed_Mode.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/vi_Frame.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

namespace Command
{


// 0 = MAPVK_VK_TO_VSC
// 1 = MAPVK_VSC_TO_VK
// 2 = MAPVK_VK_TO_CHAR
// 3 = MAPVK_VSC_TO_VK_EX
// 4 = MAPVK_VK_TO_VSC_EX
#if !defined(MAPVK_VK_TO_CHAR)
    #define MAPVK_VK_TO_CHAR 2
#endif

// 0x000...0x0FF    Graphic key
// 0x100...0x1FF    Non-Graphic Key
// 0x200...0x2FF    Ctrl+Graphic
// 0x300...0x3FF    Ctrl+Non-Graphic Key
// 0x400...0x4FF
// 0x500...0x5FF    Shift+Non-Graphic Key
// 0x600...0x6FF    Ctrl+Shift+Graphic Key
// 0x700...0x7FF    Ctrl+Shift+Non-Graphic Key

// [E]
/// <summary>
///  Dynamic Abbrevation
/// </summary>
class DynamicAbbrev :
    public Buffer::Property
{
    public: static const char* const k_pszName;

    private: enum Dir
    {
        Dir_Backward,
        Dir_Forward,
    }; // Dir

    private: typedef text::Range Range;

    private: Count          m_cwch;
    private: bool           m_fFound;
    private: Dir            m_eDir;
    private: Posn           m_lStart;
    private: Selection*     m_pSelection;
    private: Frame*         m_pFrame;
    private: Range*         m_pRange;
    private: char16         m_wsz[10];

    public: DynamicAbbrev(
        text::Buffer* pBuffer) :
            Buffer::Property(k_pszName),
            m_pRange(pBuffer->CreateRange())
    {
        // Nothing to do
    } // DynamicAbbrev

    public: void Run(const Context& context)
    {
        m_pFrame     = context.GetFrame();
        m_pSelection = context.GetSelection();

        if (NULL == m_pSelection)
        {
            return;
        }

        if (Selection_None != m_pSelection->GetType())
        {
            return;
        }

        if (! isContinue(context))
        {
            if (!start())
            {
                return;
            }
        }

        expand();
    } // Run

    // [E]
    private: void expand()
    {
        const auto current_word = m_pRange->GetText();

        while (findWord())
        {
            const auto word = m_pRange->GetText();

            if (current_word != word)
            {
                m_pSelection->SetStart(m_lStart);
                m_pSelection->SetText(word.data(),
                                      static_cast<int>(word.length()));
                m_pSelection->Collapse(Collapse_End);
                m_fFound = true;
                break;
            }
        } // while
    } // expand

    // [F]
    private: bool findWord()
    {
        switch (m_eDir)
        {
        case Dir_Backward:
            if (findWordAux(-1))
            {
                return true;
            }

            m_eDir = Dir_Forward;

            m_pRange->SetRange(
                m_pSelection->GetEnd(),
                m_pSelection->GetEnd() );

            m_pFrame->ShowMessage(
                MessageLevel_Warning,
                IDS_DA_CONTINUE,
                m_wsz );
            // FALLTHROUGH

        case Dir_Forward:
            if (findWordAux(1))
            {
                return true;
            }

            m_eDir = Dir_Backward;
            m_pRange->SetRange(m_lStart, m_lStart);

            m_pFrame->ShowMessage(
                MessageLevel_Warning,
                static_cast<uint>(
                    m_fFound ? IDS_DA_NO_MORE : IDS_DA_NOT_FOUND),
                m_wsz );
            return false;

        default:
            CAN_NOT_HAPPEN();
        } // switch dir
    } // findWord

    private: bool findWordAux(Count const k)
    {
        for (;;)
        {
            if (k > 0)
            {
                m_pRange->Collapse(Collapse_End);
                if (0 == m_pRange->MoveEnd(Unit_Word, 1))
                {
                    return false;
                }
            }
            else
            {
                m_pRange->Collapse(Collapse_Start);
                if (0 == m_pRange->MoveStart(Unit_Word, -1))
                {
                    return false;
                }
            }

            Count const cwch = m_pRange->GetEnd() - m_pRange->GetStart();
            if (cwch > m_cwch)
            {
                if (matchPrefix(m_pRange))
                {
                    return true;
                }
            }
        } // for
    } // findWordAux

    // [I]
    private: bool isContinue(const Context& context)
    {
        if (context.GetThisCommand()  != context.GetLastCommand())
        {
            return false;
        }

        ASSERT(m_pRange->GetBuffer() == m_pSelection->GetBuffer());

        if (m_pSelection->GetStart() < m_lStart)
        {
            return false;
        }

        Range oRange(*m_pSelection);
        oRange.MoveStart(Unit_Word, -1);
        return matchPrefix(&oRange);
    } // isContinue

    private: bool matchPrefix(Range* const pRange)
    {
        Count k = 0;
        foreach (Buffer::EnumChar, oEnum, pRange)
        {
            if (oEnum.Get() != m_wsz[k])
            {
                return false;
            }

            k += 1;

            if (k == m_cwch)
            {
                return true;
            }
        } // for each char
        return false;
    } // matchPrefix

    // [S]
    private: bool start()
    {
        Range oRange(*m_pSelection);
        oRange.MoveStart(Unit_Word, -1);
        Count cwch = oRange.GetEnd() - oRange.GetStart();
        if (cwch > lengthof(m_wsz) - 1)
        {
            m_pFrame->ShowMessage(
                MessageLevel_Warning,
                IDS_DA_TOO_LONG );
            return false;
        }

        if (cwch <= 1)
        {
            m_pFrame->ShowMessage(
                MessageLevel_Warning,
                IDS_DA_TOO_SHORT );
            return false;
        }

        m_cwch = cwch;
        char16* pwsz = m_wsz;
        foreach (Buffer::EnumChar, oEnum, &oRange)
        {
            *pwsz++ = oEnum.Get();
        } // for each char
        *pwsz = 0;

        Posn const lStart = oRange.GetStart();

        m_eDir    = Dir_Backward;
        m_fFound  = false;
        m_lStart  = lStart;

        m_pRange->SetRange(lStart, lStart);

        return true;
    } // start

    DISALLOW_COPY_AND_ASSIGN(DynamicAbbrev);
}; // DynamicAbbrev

const char* const DynamicAbbrev::k_pszName = "DynamicAbbrev";

DEFCOMMAND(ExpandDynamicAbbrev)
{
    text::Buffer* pBuffer = pCtx->GetSelection()->GetBuffer();

    DynamicAbbrev* pDynamicAbbrev = pBuffer->FindProperty<DynamicAbbrev>(
        DynamicAbbrev::k_pszName);

    if (NULL == pDynamicAbbrev)
    {
        pDynamicAbbrev = new(pBuffer->GetHeap()) DynamicAbbrev(pBuffer);
        pBuffer->PutProperty(pDynamicAbbrev);
    }

    pDynamicAbbrev->Run(*pCtx);
} // ExpandDynamicAbbrev

DEFCOMMAND(ExchangeCode)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;

    text::Range oRange(*pSelection);

    if (oRange.GetStart() == oRange.GetEnd())
    {
        oRange.MoveStart(Unit_Word, -1);

        if (oRange.GetEnd() - oRange.GetStart() > 4)
        {
            oRange.SetStart(oRange.GetEnd() - 4);
        }
    }
    else if (oRange.GetEnd() - oRange.GetStart() > 4)
    {
        return;
    }

    int cDigits = 0;
    int nCode   = 0;
    int iChar   = -1;
    int iTimes  = 1;
    foreach (Buffer::EnumCharRev, oEnum, &oRange)
    {
        char16 wch = oEnum.Get();

        if (iChar < 0)
        {
            iChar = wch;
        }

        if (wch >= '0' && wch <= '9')
        {
            nCode += (wch - '0') * iTimes;
        }
        else if (wch >= 'A' && wch <= 'F')
        {
            nCode += (wch - 'A' + 10) * iTimes;
        }
        else if (wch >= 'a' && wch <= 'f')
        {
            nCode += (wch - 'a' + 10) * iTimes;
        }
        else
        {
            break;
        }
        
        iTimes <<= 4;
        cDigits += 1;
    } // for

    class Util
    {
        public: static char16 toxdigit(int x)
        {
            if (x >= 0 && x <= 9)   return static_cast<char16>(x + '0');
            if (x >= 10 && x <= 15) return static_cast<char16>(x + 'A' - 10);
            return '_';
        } // toxdigit
    }; // Util

    char16 rgwch[4];
    int cwch;

    if (cDigits <= 1)
    {
        // Convert the last character to Unicode.
        oRange.SetStart(oRange.GetEnd() - 1);
        rgwch[0] = Util::toxdigit((iChar >> 12) & 15);
        rgwch[1] = Util::toxdigit((iChar >>  8) & 15);
        rgwch[2] = Util::toxdigit((iChar >>  4) & 15);
        rgwch[3] = Util::toxdigit((iChar >>  0) & 15);
        cwch = 4;
    }
    else
    {
        oRange.SetStart(oRange.GetEnd() - cDigits);
        cwch = 1;
        rgwch[0] = static_cast<char16>(nCode);
    }

    oRange.SetText(base::string16(rgwch, static_cast<size_t>(cwch)));
    pSelection->SetRange(oRange.GetEnd(), oRange.GetEnd());
} // ExchangeCode

// [G]
DEFCOMMAND(GoToCloseParen)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;
    Posn lPosn = pSelection->FindCloseParen(true);
    when (-1 != lPosn) pSelection->SetRange(lPosn, lPosn);
} // GoToCloseParen

DEFCOMMAND(GoToCloseParenExtend)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;
    Posn lPosn = pSelection->FindCloseParen(true);
    when (-1 != lPosn)
    {
        pSelection->SetEnd(lPosn);
        pSelection->SetStartIsActive(false);
    }
} // GoToCloseParenExtend

DEFCOMMAND(GoToOpenParen)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;
    Posn lPosn = pSelection->FindOpenParen(true);
    when (-1 != lPosn) pSelection->SetRange(lPosn, lPosn);
} // GoToOpenParen

DEFCOMMAND(GoToOpenParenExtend)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;
    Posn lPosn = pSelection->FindOpenParen(true);
    when (-1 != lPosn)
    {
        pSelection->SetStart(lPosn);
        pSelection->SetStartIsActive(true);
    }
} // GoToOpenParenExtend

// [P]

DEFCOMMAND(PasteFromClipboard)
{
    if (NULL == pCtx->GetSelection()) return;
    pCtx->GetSelection()->Paste();
} // PasteFromClipboard

// [Q]
DEFCOMMAND(QuotedInsert)
{
    ASSERT(NULL != pCtx);
    // This is dummy entry of QuotedInsert command.
    // This command is handled by command processor.
} // StartArgument

// [R]

DEFCOMMAND(Reconvert)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;

    pSelection->GetWindow()->Reconvert(
        pSelection->GetStart(),
        pSelection->GetEnd() );
} // Reconvert

DEFCOMMAND(Reload)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;

    Buffer* pBuffer = pSelection->GetBuffer();

    unless (pBuffer->NeedSave()) return;

    int iAnswer;
    {
      UI_DOM_AUTO_UNLOCK_SCOPE();
      iAnswer = Application::instance()->Ask(
          MB_YESNO | MB_ICONQUESTION,
          IDS_ASK_RELOAD,
          pBuffer->GetName() );
    }

    if (IDYES == iAnswer)
    {
        pBuffer->Reload();
    }
} // Reload

// [S]
DEFCOMMAND(StartArgument)
{
    ASSERT(NULL != pCtx);
    // This is dummy entry of StartArgument command.
    // This command is handled by command processor.
} // StartArgument

// [T]

/// <summary>
///    TypeChar command
/// </summary>
void TypeChar(const Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();
    if (NULL == pSelection) return;

    char16 wch = pCtx->GetLastChar();
    Count  n   = pCtx->GetArg();

    pSelection->TypeChar(wch, n);

    uint nTrait = pSelection->GetBuffer()->GetMode()->GetCharSyntax(wch);
    if (text::CharSyntax::Syntax_CloseParen ==
        text::CharSyntax::GetSyntax(nTrait) )
    {
        // Force color newly inserted characters.
        // Note: If we are in long comment, parenthesis matching may not
        // work.
        pSelection->GetBuffer()->OnIdle(100);

        Posn lOpen = pSelection->FindOpenParen();

        if (lOpen >= 0)
        {
            // Blink 100 msec if matched open parenthesis in window,
            // otherwise blink 500ms
            int nWait = 100;

            if (lOpen < pSelection->GetWindow()->GetStart())
            {
                nWait = 500;
            }

            pSelection->Blink(lOpen, nWait);
        }
        else if (pSelection->GetStart() != lOpen)
        {
            Application::instance()->GetActiveFrame()->ShowMessage(
                MessageLevel_Warning,
                IDS_NO_MATCHING_PAREN );
        }
    } // if close paren
} // TypeChar

static char16 s_rgwchGraphKey[256];

static const char16* const s_rgpwszVKeyName[256] =
{
    #define KeyName(x) L ## #x
    L"00",
    KeyName(LBUTTON),   // 0x01
    KeyName(RBUTTON),   // 0x02
    KeyName(CANCEL),    // 0x03
    KeyName(MBUTTON),   // 0x04    /* NOT contiguous with L & RBUTTON */
    KeyName(XBUTTON1),  // 0x05    /* NOT contiguous with L & RBUTTON */
    KeyName(XBUTTON2),  // 0x06    /* NOT contiguous with L & RBUTTON */
    L"07",
    KeyName(Backspave), // 0x08 BACK
    KeyName(Tab),       // 0x09
    L"0A",
    L"0B",
    KeyName(Clear),     // 0x0C
    KeyName(Enter),     // 0x0D RETURN
    L"0E",
    L"0F",

    KeyName(SHIFT),     // 0x10
    KeyName(CONTROL),   // 0x11
    KeyName(MENU),      // 0x12
    KeyName(Pause),     // 0x13
    KeyName(Capital),   // 0x14
    KeyName(KANA),      // 0x15 HANGEUL/NAGUL
    L"16",
    KeyName(JUNJA),     // 0x17
    KeyName(FINAL),     // 0x18
    KeyName(KANJI),     // 0x19 HANJA
    L"1A",
    KeyName(Escape),    // 0x1B
    KeyName(CONVERT),   // 0x1C
    KeyName(NONCONVERT),// 0x1D
    KeyName(ACCEPT),    // 0x1E
    KeyName(MODECHANGE),// 0x1F

    KeyName(Space),     // 0x20
    KeyName(PageUp),    // 0x21 PRIOR
    KeyName(PageDn),    // 0x22 NEXT
    KeyName(End),       // 0x23
    KeyName(Home),      // 0x24
    KeyName(Left),      // 0x25
    KeyName(Up),        // 0x26
    KeyName(Right),     // 0x27
    KeyName(Down),      // 0x28
    KeyName(Select),    // 0x29
    KeyName(Print),     // 0x2A
    KeyName(EXECUTE),   // 0x2B
    KeyName(SNAPSHOT),  // 0x2C
    KeyName(Insert),    // 0x2D
    KeyName(Delete),    // 0x2E
    KeyName(Help),      // 0x2F

    L"30", L"31", L"32", L"33", L"34", L"35", L"36", L"37",
    L"38", L"39", L"3A", L"3B", L"3C", L"3D", L"3E", L"3F",

    L"40", L"41", L"42", L"43", L"44", L"45", L"46", L"47",
    L"48", L"49", L"4A", L"4B", L"4C", L"4D", L"4E", L"4F",

    L"50", L"51", L"52", L"53", L"54", L"55", L"56", L"57",
    L"58", L"59", L"5A",
    KeyName(LWIN),      // 0x5B
    KeyName(RWIN),      // 0x5C
    KeyName(APPS),      // 0x5D
    L"5E",
    KeyName(SLEEP),     // 0x5F

    KeyName(NumPad0),   // 0x60
    KeyName(NumPad1),   // 0x61
    KeyName(NumPad2),   // 0x62
    KeyName(NumPad3),   // 0x63
    KeyName(NumPad4),   // 0x64
    KeyName(NumPad5),   // 0x65
    KeyName(NumPad6),   // 0x66
    KeyName(NumPad7),   // 0x67
    KeyName(NumPad8),   // 0x68
    KeyName(NumPad9),   // 0x69
    KeyName(MULTIPLY),  // 0x6A
    KeyName(ADD),       // 0x6B
    KeyName(SEPARATOR), // 0x6C
    KeyName(SUBTRACT),  // 0x6D
    KeyName(DECIMAL),   // 0x6E
    KeyName(DIVIDE),    // 0x6F

    KeyName(F1),    // 0x70
    KeyName(F2),    // 0x71
    KeyName(F3),    // 0x72
    KeyName(F4),    // 0x73
    KeyName(F5),    // 0x74
    KeyName(F6),    // 0x75
    KeyName(F7),    // 0x76
    KeyName(F8),    // 0x77
    KeyName(F9),    // 0x78
    KeyName(F10),   // 0x79
    KeyName(F11),   // 0x7A
    KeyName(F12),   // 0x7B
    KeyName(F13),   // 0x7C
    KeyName(F14),   // 0x7D
    KeyName(F15),   // 0x7E
    KeyName(F16),   // 0x7F
    KeyName(F17),   // 0x80
    KeyName(F18),   // 0x81
    KeyName(F19),   // 0x82
    KeyName(F20),   // 0x83
    KeyName(F21),   // 0x84
    KeyName(F22),   // 0x85
    KeyName(F23),   // 0x86
    KeyName(F24),   // 0x87
    L"88", L"89", L"8A", L"8B", L"8C", L"8D", L"8E", L"8F",

    KeyName(NumLoCK),   // 0x90
    KeyName(ScrLk),     // 0x91 SCROLL

    // NEC PC-9800 kbd definitions
    //KeyName(OEM_NEC_EQUAL), //  0x92   // '=' key on numpad

    // Fujitsu/OASYS kbd definitions
    KeyName(OEM_FJ_JISHO),  // 0x92   // 'Dictionary' key
    KeyName(OEM_FJ_MASSHOU),// 0x93   // 'Unregister word' key
    KeyName(OEM_FJ_TOUROKU),// 0x94   // 'Register word' key
    KeyName(OEM_FJ_LOYA),   // 0x95   // 'Left OYAYUBI' key
    KeyName(OEM_FJ_ROYA),   // 0x96   // 'Right OYAYUBI' key
    L"97",
    L"98", L"99", L"9A", L"9B", L"9C", L"9D", L"9E", L"9F",

    // VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
    // Used only as parameters to GetAsyncKeyState() and GetKeyState().
    // No other API or message will distinguish left and right keys in this
    // way.
    KeyName(LSHIFT),    // 0xA0
    KeyName(RSHIFT),    // 0xA1
    KeyName(LCONTROL),  // 0xA2
    KeyName(RCONTROL),  // 0xA3
    KeyName(LMENU),     // 0xA4
    KeyName(RMENU),     // 0xA5
    KeyName(BROWSER_BACK),      // 0xA6
    KeyName(BROWSER_FORWARD),   // 0xA7
    KeyName(BROWSER_REFRESH),   // 0xA8
    KeyName(BROWSER_STOP),      // 0xA9
    KeyName(BROWSER_SEARCH),    // 0xAA
    KeyName(BROWSER_FAVORITES), // 0xAB
    KeyName(BROWSER_HOME),      // 0xAC
    KeyName(VOLUME_MUTE),       // 0xAD
    KeyName(VOLUME_DOWN),       // 0xAE
    KeyName(VOLUME_UP),         // 0xAF

    KeyName(MEDIA_NEXT_TRACK),  // 0xB0
    KeyName(MEDIA_PREV_TRACK),  // 0xB1
    KeyName(MEDIA_STOP),        // 0xB2
    KeyName(MEDIA_PLAY_PAUSE),  // 0xB3
    KeyName(LAUNCH_MAIL),       // 0xB4
    KeyName(LAUNCH_MEDIA_SELECT), // 0xB5
    KeyName(LAUNCH_APP1),       // 0xB6
    KeyName(LAUNCH_APP2),       // 0xB7
    L"B8", L"B9",
    KeyName(OEM_1),         // 0xBA   // ';:' for US
    KeyName(OEM_PLUS),      // 0xBB   // '+' any country
    KeyName(OEM_COMMA),     // 0xBC   // ',' any country
    KeyName(OEM_MINUS),     // 0xBD   // '-' any country
    KeyName(OEM_PERIOD),    // 0xBE   // '.' any country
    KeyName(OEM_2),         // 0xBF   // '/?' for US

    KeyName(OEM_3),         // 0xC0   // '`~' for US
           L"C1", L"C2", L"C3", L"C4", L"C5", L"C6", L"C7",
    L"C8", L"C9", L"CA", L"CB", L"CC", L"CD", L"CE", L"CF",

    L"D0", L"D1", L"D2", L"D3", L"D4", L"D5", L"D6", L"D7",
    L"D8", L"D9", L"DA",
    KeyName(OEM_4), // 0xDB  //  '[{' for US
    KeyName(OEM_5), // 0xDC  //  '\|' for US
    KeyName(OEM_6), // 0xDD  //  ']}' for US
    KeyName(OEM_7), // 0xDE  //  ''"' for US
    KeyName(OEM_8), // 0xDF

    L"E0",
    KeyName(OEM_AX),    // 0xE1  //  'AX' key on Japanese AX kbd
    KeyName(OEM_102),   // 0xE2  //  "<>" or "\|" on RT 102-key kbd.
    KeyName(ICO_HELP),  // 0xE3  //  Help key on ICO
    KeyName(ICO_00),    // 0xE4  //  00 key on ICO
    KeyName(PROCESSKEY), // 0xE5
    KeyName(ICO_CLEAR), // 0xE6
    KeyName(PACKET),    // 0xE7
    L"E8",
    // Nokia/Ericsson definitions
    KeyName(OEM_RESET), // 0xE9
    KeyName(OEM_JUMP),  // 0xEA
    KeyName(OEM_PA1),   // 0xEB
    KeyName(OEM_PA2),   // 0xEC
    KeyName(OEM_PA3),   // 0xED
    KeyName(OEM_WSCTRL),// 0xEE
    KeyName(OEM_CUSEL), // 0xEF

    KeyName(OEM_ATTN),  // 0xF0
    KeyName(OEM_FINISH),// 0xF1
    KeyName(OEM_COPY),  // 0xF2
    KeyName(OEM_AUTO),  // 0xF3
    KeyName(OEM_ENLW),  // 0xF4
    KeyName(OEM_BACKTAB),//0xF5
    KeyName(ATTN),      // 0xF6
    KeyName(CRSEL),     // 0xF7
    KeyName(EXSEL),     // 0xF8
    KeyName(EREOF),     // 0xF9
    KeyName(PLAY),      // 0xFA
    KeyName(ZOOM),      // 0xFB
    KeyName(NONAME),    // 0xFC
    KeyName(PA1),       // 0xFD
    KeyName(OEM_CLEAR), // 0xFE
    L"FF",
}; // s_rgpwszVKeyName

static char16 k_rgwszKeyName[256*2];

const char16* GetKeyName(uint nKey)
{
    nKey &= 0x1FF;
    if (nKey <= 0xFF)
    {
        if (0x20 == nKey) return s_rgpwszVKeyName[VK_SPACE];
        return &k_rgwszKeyName[nKey * 2];
    }

    uint nVKey = nKey - 0x100;
    uint nChar = s_rgwchGraphKey[nVKey];
    if (0 != nChar)
    {
        return &k_rgwszKeyName[nChar * 2];
    }

    return s_rgpwszVKeyName[nVKey];
} // GetKeyName


//////////////////////////////////////////////////////////////////////
//
// TranslateKey
//
uint TranslateKey(uint nVKey)
{
    if (VK_CONTROL == nVKey) return 0;
    if (VK_SHIFT   == nVKey) return 0;

    uint nKey = s_rgwchGraphKey[nVKey];

    if (0 == nKey)
    {
        nKey = nVKey | 0x100;
    }

    if (::GetKeyState(VK_CONTROL) < 0)
    {
        nKey |= Mod_Ctrl;
    }
    else if (nKey <= 0xFF)
    {
        return 0;
    }

    if (::GetKeyState(VK_SHIFT) < 0)  
    {
        nKey |= Mod_Shift;
    }
    return nKey;
} // TranslateKey

namespace {

class StaticKeyBindEntry {
  private: common::scoped_refptr<KeyBindEntry> entry_;
  public: StaticKeyBindEntry(Command::CommandFn function)
      : entry_(new Command(function)) {
  }
  public: ~StaticKeyBindEntry() = default;
  public: KeyBindEntry* get() const { return entry_.get(); }
};

}  // namespace

KeyBindEntry* QuotedInsertEntry() {
  DEFINE_STATIC_LOCAL(StaticKeyBindEntry, entry, (QuotedInsert));
  return entry.get();
}

KeyBindEntry* StartArgumentEntry() {
  DEFINE_STATIC_LOCAL(StaticKeyBindEntry, entry, (StartArgument));
  return entry.get();
}

//////////////////////////////////////////////////////////////////////
//
// Processor::GlobalInit
//
void Processor::GlobalInit() {
    for (uint nKey = 0; nKey <= 255; nKey++)
    {
        k_rgwszKeyName[nKey*2] = static_cast<char16>(nKey);
    } // for

    for (uint nVKey = 0; nVKey <= 255; nVKey++)
    {
        uint nChar = ::MapVirtualKey(nVKey, MAPVK_VK_TO_CHAR);

        if (nChar >= 0x20)
        {
            s_rgwchGraphKey[nVKey] = static_cast<char16>(nChar);
        }
    } // for each nVKey

    // Self Insert
    {
      common::scoped_refptr<KeyBindEntry> self_insert_entry(
          new Command(TypeChar));
      for (int nKey = 0x20; nKey < 0x7F; nKey++) {
        TextEditWindow::BindKey(nKey, self_insert_entry);
      }
    }

    #define BIND_KEY(mp_class, mp_keycomb, mp_cmd) \
    { \
        mp_class::BindKey(mp_keycomb, mp_cmd); \
    } // BIND_CTRL

    #define BIND_VKEY(mp_class, mp_mod, mp_keycomb, mp_cmd) \
    { \
        mp_class::BindKey( \
            mp_mod | VK_ ## mp_keycomb | 0x100, \
            mp_cmd ); \
    } // BIND_CTRL

    // Ctrl
    BIND_KEY(TextEditWindow, Mod_Ctrl | ',', Reconvert);
    BIND_KEY(TextEditWindow, Mod_Ctrl | '.', ExchangeCode);
    BIND_KEY(TextEditWindow, Mod_Ctrl | '/', ExpandDynamicAbbrev);

    BIND_KEY(TextEditWindow, Mod_Ctrl | 'Q', QuotedInsertEntry());
    BIND_KEY(TextEditWindow, Mod_Ctrl | 'R', Reload);
    BIND_KEY(CommandWindow, Mod_Ctrl | 'U', StartArgumentEntry());

    BIND_VKEY(TextEditWindow, Mod_Ctrl, DOWN,   GoToCloseParen);
    BIND_VKEY(TextEditWindow, Mod_Ctrl, UP,     GoToOpenParen);

    BIND_VKEY(TextEditWindow, Mod_CtrlShift, DOWN,      GoToCloseParenExtend);
    BIND_VKEY(TextEditWindow, Mod_CtrlShift, UP,        GoToOpenParenExtend);
} // Processor::GlobalInit

}  // namespace Command

void Buffer::BindKey(uint key_code, Command::KeyBindEntry* entry) {
  if (!key_bindings_)
    key_bindings_.reset(new Command::KeyBinds());
  key_bindings_->Bind(static_cast<int>(key_code), entry);
}

Command::KeyBindEntry* Buffer::MapKey(uint key_code) const {
  auto const key_bindings = key_bindings_.get();
  if (!key_bindings)
    return nullptr;
  return key_bindings->MapKey(static_cast<int>(key_code));
}
