#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - listener pane
// listener/winapp/ap_listener_pane.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_listener_buffer.cpp#1 $
//
#include "./ap_listener_buffer.h"

#include "./cm_CmdProc.h"

#include "./vi_Selection.h"
#include "./vi_Style.h"
#include "./vi_TextEditWindow.h"

#include "./listener.h"

namespace {
ListenerBuffer* ToListenerBuffer(Buffer* buffer) {
  ASSERT(buffer);
  // warning C4946: reinterpret_cast used between related classes: 
  // 'class1' and 'class2'
  #pragma warning(suppress: 4946)
  return reinterpret_cast<ListenerBuffer*>(buffer);
}
}

Command::KeyBinds* ListenerBuffer::sm_pKeyBinds;

void Listener_BackwardLine(const Command::Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();

    auto const pListener = ToListenerBuffer(pSelection->GetBuffer());

    Count lCount = pCtx->GetArg();

    Posn lOutputEnd = pListener->m_oGateway.GetOutput()->GetEnd();

    if (pSelection->GetStart() == lOutputEnd &&
        pSelection->GetEnd()   == lOutputEnd )
    {
        pListener->m_oHistory.Backward(pSelection);
        pSelection->SetRange(lOutputEnd, lOutputEnd);
    }
    else
    {
        pSelection->MoveUp(Unit_Line, lCount);
    } // if
} // Listener_BackwardLine


void Listener_CtrlC(const Command::Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();

    if (pSelection->GetType() == Selection_None )
    {
        auto const pListener = ToListenerBuffer(pSelection->GetBuffer());

        pListener->Interrupt();
    }
    else
    {
        pSelection->Copy();
    } // if
} // Listener_CtrlC

void Listener_Enter(const Command::Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();

    auto const pListener = ToListenerBuffer(pSelection->GetBuffer());

    #if 0
    // Listener doesn't accept user input until listener gets output
    // from subprocess.
    ::SendMessage(
        *GetFrame(),
        FRAME_WM_SETSTATUS,
        0,
        reinterpret_cast<LPARAM>(L"Busy") );
    #endif

    Posn lEnd = pSelection->GetBuffer()->GetEnd();
    pSelection->SetRange(lEnd, lEnd);
    pSelection->TypeChar(0x0A);

    pListener->SendText();
} // Listener_Enter

void Listener_ForwardLine(const Command::Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();
    Count lCount = pCtx->GetArg();

    auto const pListener = ToListenerBuffer(pSelection->GetBuffer());

    Posn lOutputEnd = pListener->m_oGateway.GetOutput()->GetEnd();

    if (pSelection->GetStart() == lOutputEnd &&
        pSelection->GetEnd()   == lOutputEnd )
    {
        pListener->m_oHistory.Forward(pSelection);
        pSelection->SetRange(lOutputEnd, lOutputEnd);
    }
    else
    {
        pSelection->MoveDown(Unit_Line, lCount);
    } // if
} // Listener_ForwardLine


void Listener_TypeChar(const Command::Context* pCtx)
{
    Selection* pSelection = pCtx->GetSelection();
    //Count lCount = pCtx->GetArg();

    auto const pListener = ToListenerBuffer(pSelection->GetBuffer());

    pListener->AdjustSelection(pSelection);
    Command::TypeChar(pCtx);
} // Listener_TypeChar


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer ctor
//
ListenerBuffer::ListenerBuffer() :
    Buffer(L"*listener*", NULL)
{
    SetNoSave(true);

    // We don't accept user input until we receive output from
    // subprocess.
    SetReadOnly(true);

    if (NULL == sm_pKeyBinds)
    {
        sm_pKeyBinds = new Command::KeyBinds;

        sm_pKeyBinds->Bind(
            Mod_Ctrl | 'C',
            new Command::Command(Listener_CtrlC) );

        sm_pKeyBinds->Bind(
            Command::MapVKey(VK_RETURN),
            new Command::Command(Listener_Enter) );

        sm_pKeyBinds->Bind(
            Command::MapVKey(VK_DOWN),
            new Command::Command(Listener_ForwardLine) );

        sm_pKeyBinds->Bind(
            Command::MapVKey(VK_UP),
            new Command::Command(Listener_BackwardLine) );

        Command::Command* pTypeCharCmd =
            new Command::Command(Listener_TypeChar);

        for (uint wch = 0x20; wch <= 0x7E; wch++)
        {
            sm_pKeyBinds->Bind(wch, pTypeCharCmd);
        } // for
    } // if
} // ListenerBuffer::ListenerBuffer


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer::Interrupt
//
void ListenerBuffer::Interrupt()
{
    m_oLispThread.Interrupt();
} // ListenerBuffer::Interrupt


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer::OnIdle
//
bool ListenerBuffer::OnIdle()
{
    return false;
} // ListenerBuffer::OnIdle


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer::SendText
//
// Description:
//  Sends characters between end of output to end of buffer to
//  subprocess.
//
void ListenerBuffer::SendText()
{
    Posn lStart = m_oGateway.GetOutput()->GetEnd();
    Posn lEnd   = GetEnd();

    // -1 for Newline.
    m_oHistory.Add(this, lStart, lEnd - 1);

    StyleValues oStyle;

    oStyle.m_rgfMask = 
        StyleValues::Mask_Color |
        StyleValues::Mask_FontStyle;

    oStyle.m_crColor    = RGB(0x55, 0x00, 0x00);
    oStyle.m_eFontStyle = FontStyle_Italic;

    SetStyle(lStart, lEnd - 1, &oStyle);

    m_oGateway.SendText();
} // ListenerBuffer::SendText


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer::Start
//
uint ListenerBuffer::Start()
{
    uint nError;

    nError = m_oLispThread.Init();
    when (nError) return nError;

    nError = m_oGateway.Start(this, m_oLispThread.GetId());
    when (nError) return nError;

    nError = m_oLispThread.Start(m_oGateway.GetHWND());
    when (nError) return nError;

    return 0;
} // ListenerBuffer::Start


//////////////////////////////////////////////////////////////////////
//
// ListenerBuffer::AdjustSelection
//
void ListenerBuffer::AdjustSelection(Selection* pSelection)
{
    if (pSelection->GetStart() < m_oGateway.GetOutput()->GetStart())
    {
        pSelection->SetRange(
            GetEnd(),
            GetEnd() );
    } // if
} // ListenerBuffer::AdjustSelection


Command::KeyBindEntry* ListenerBuffer::MapKey(uint nKey) const
{
    Command::KeyBindEntry* pEntry = sm_pKeyBinds->MapKey(nKey);
    if (pEntry != NULL) return pEntry;
    return Command::g_pGlobalBinds->MapKey(nKey);
} // ListenerBuffer::MapKey

