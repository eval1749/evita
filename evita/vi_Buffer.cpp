#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Edit Buffer
// listener/winapp/vi_buffer.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Buffer.cpp#1 $
//
#include "./vi_Buffer.h"

#include "./ed_Mode.h"

#include "evita/editor/application.h"
#include "./vi_Frame.h"

//////////////////////////////////////////////////////////////////////
//
// Buffer::CanKill
//  Returns true if buffer is safe to kill.
bool Buffer::CanKill()
{
    unless (NeedSave()) return true;

    char16 wsz[1024];
    ::wsprintf(wsz,
        L"Do you want to save the changes to %s?",
        GetName() );

    Frame* pFrame = Application::instance()->GetActiveFrame();

    int iAnswer = Application::instance()->Ask(
        MB_ICONWARNING | MB_YESNOCANCEL,
        IDS_ASK_SAVE,
        GetName() );

    switch (iAnswer)
    {
    case IDCANCEL:
        return false;

    case IDYES:
        if (! Application::instance()->SaveBuffer(pFrame, this))
        {
            return false;
        }
        break;
    } // switch

    return true;
} // Buffer::CanKill


//////////////////////////////////////////////////////////////////////
//
// Buffer::GetWindow
//  Returns MRU window
Buffer::Window* Buffer::GetWindow() const
{
    const auto* mru = m_oWindows.GetFirst();
    for (auto& window: m_oWindows) {
        if (mru->GetActiveTick() < window.GetActiveTick())
            mru = &window;
    } // for each window
    return const_cast<Window*>(mru);
}

//////////////////////////////////////////////////////////////////////
//
// Buffer::OnIdle
//
bool Buffer::OnIdle(uint)
{
    #if DEBUG_STYLE
    {
        DEBUG_PRINTF(L"%p\n", this);
        int nNth = 0;
        foreach (EnumInterval, oEnum, this)
        {
            Interval* pIntv = oEnum.Get();
            DEBUG_PRINTF(L"%d [%d, %d] #%06X\n", 
                nNth, pIntv->GetStart(), pIntv->GetEnd(),
                pIntv->GetStyle()->GetColor() );
            nNth += 1;
        } // for each interval
    }
    #endif // DEBUG_STYLE

    // Note: If we decrase number for coloring amount, we'll see
    // more redisplay.
    return GetMode()->DoColor(500);
} // Buffer::OnIdle
