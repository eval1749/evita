#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - FontSet class
// listener/winapp/fontset.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/cm_CmdProc.cpp#2 $
//
#include "./cm_CmdProc.h"

#include "./ed_util.h"

#include "evita/editor/application.h"
#include "./vi_Buffer.h"
#include "./vi_EditPane.h"
#include "./vi_Selection.h"
#include "./vi_TextEditWindow.h"

namespace Command
{

KeyBindEntry* g_pQuotedInsert;
KeyBindEntry* g_pStartArgument;

/// <summary>
///  Construct KeyBinds object
/// </summary>
KeyBinds::KeyBinds()
{
    myZeroMemory(m_rgpBind, sizeof(m_rgpBind));
} // KeyBinds::KeyBinds

/// <summary>
///   Advance command processing state with specified keyboard event.
/// </summary>
/// <param name="fRepeat">True if keyboard repeat event.</param>
/// <param name="nKey">Key code.</param>
/// <param name="pWindow">A window object causing an event.</param>
void Processor::Execute(
    CommandWindow*  pWindow,
    uint            nKey,
    uint            fRepeat )
{
    KeyBindEntry* pEntry = NULL;

    m_pWindow = pWindow;
    m_wchLast = static_cast<char16>(nKey);

    TextEditWindow* pEditWindow = pWindow->DynamicCast<TextEditWindow>();

    if (NULL == pEditWindow)
    {
        m_pFrame     = Application::Get()->GetActiveFrame();
        m_pSelection = NULL;
    }
    else
    {
        m_pSelection = pEditWindow->GetSelection();

        m_pFrame = m_pSelection->GetWindow()->container_widget().
            as<EditPane>()->GetFrame();
    }

    m_rgnKey[m_cKeys] = nKey;
    m_cKeys += 1;

    switch (m_eState)
    {
    case State_Arg:
        if (nKey >= '0' && nKey <= '9')
        {
            m_iArg *= 10;
            m_iArg += nKey - '0';
            return;
        }

        m_iArg *= m_iSign;

        pEntry = pWindow->MapKey(nKey);
        break;

    case State_Continue:
        pEntry = pWindow->MapKey(nKey);
        break;

    case State_Quote:
        if (NULL != m_pSelection)
        {
            // Insert ASCII character into buffer.
            if (nKey >= (Mod_Ctrl | 'A') && nKey <= (Mod_Ctrl | 0x5F))
            {
                nKey &= 0x1F;
            }

            if (nKey <= 0x7F)
            {
                m_pSelection->TypeChar(static_cast<char16>(nKey));
            }
        }
        Reset();
        return;

    case State_StartArg:
        if ('-' == nKey)
        {
            m_eState = State_Arg;
            m_iSign  = -1;
            return;
        } // if

        if (nKey >= '0' && nKey <= '9')
        {
            m_eState = State_Arg;
            m_iArg = static_cast<Count>(nKey - '0');
            return;
        } // if

        m_iArg = 4;

        pEntry = pWindow->MapKey(nKey);
        break;

    case State_Start:
        m_pFrame->ResetMessages();

        pEntry = pWindow->MapKey(nKey);

        m_iArg = fRepeat ? 2 : 1;

        if (pEntry == g_pStartArgument)
        {
            m_eState = State_StartArg;
            m_iArg  = 0;
            m_iSign = 1;
            return;
        } // if
        break;

    default:
        CAN_NOT_HAPPEN();
    } // switch state

    if (pEntry == g_pStartArgument)
    {
        m_eState = State_Continue;
        m_iArg *= 4;
        return;
    } // if

    if (pEntry == g_pQuotedInsert)
    {
        m_eState = State_Quote;
        return;
    }

    if (pEntry == NULL)
    {
        reportUnboundKeys();
        Reset();
        return;
    } // if

    switch (pEntry->GetKind())
    {
    case Bind_Command:
        m_pThisCommand = pEntry->StaticCast<Command>();
        pEntry->StaticCast<Command>()->Execute(this);
        m_pLastCommand = m_pThisCommand;
        Reset();
        break;

    case Bind_KeyBinds:
        m_pKeyBinds = pEntry->StaticCast<KeyBinds>();
        m_eState = State_Continue;
        break;

    default:
        CAN_NOT_HAPPEN();
    } // switch kind
} // Processor::Execute

void Processor::reportUnboundKeys()
{
    CharSink_<> oSink;
    for (uint i = 0; i < m_cKeys; i++)
    {
        auto const nKey = m_rgnKey[i];

        if (nKey & Mod_Ctrl)
        {
            oSink.Add(L"Ctrl+");
        }

        if (nKey & Mod_Shift)
        {
            oSink.Add(L"Shift+");
        }

        if (nKey <= 0xFF)
        {
            oSink.Add(static_cast<char16>(nKey));
        }
        else
        {
            oSink.Add(GetKeyName(nKey));
        }
    } // for

    oSink.Add(static_cast<char16>(0));

    Application::Get()->GetActiveFrame()->ShowMessage(
        MessageLevel_Warning,
        IDS_NOT_COMMAND,
        oSink.GetStart() );
} // reportUnboundKeys

/// <summary>
///   Reset command processor.
/// </summary>
void Processor::Reset()
{
    m_eState    = State_Start;
    m_cKeys     = 0;
    m_iArg      = 0;
    m_iSign     = 0;
    m_pKeyBinds = NULL;
} // Processor::Reset

} // Command
