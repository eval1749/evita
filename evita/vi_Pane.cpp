#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - winmain
// listener/winapp/winmain.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Pane.cpp#1 $
//
#include "evita/vi_Pane.h"

#include "common/win/native_window.h"
#include "evita/vi_Frame.h"

Pane::Pane(std::unique_ptr<common::win::NativeWindow>&& native_window)
    : CommandWindow_(std::move(native_window)),
      m_nActiveTick(0),
      m_pwszName(L"") {
}

Pane::Pane()
    : Pane(std::move(std::unique_ptr<common::win::NativeWindow>())) {
}

void Pane::Activate() {
  ++m_nActiveTick;
  GetFrame()->DidActivatePane(this);
}
