#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Mason Mode
// listener/winapp/mode_Mason.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Mason.cpp#4 $
//
#include "./mode_Mason.h"


namespace Edit
{
/// <summary>
///   Returns true if Mason mode support specified buffer name.
/// </summary>
bool MasonModeFactory::IsSupported(const char16* pwsz) const
{
    if (0 == lstrcmpW(pwsz, L"autohandler")) return true;
    if (0 == lstrcmpW(pwsz, L"dhandler")) return true;
    return ModeFactory::IsSupported(pwsz);
} // MasonModeFactory::IsSupported

} // Edit
