#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Edit Buffer
// listener/winapp/ed_buffer.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/li_util.cpp#1 $
//
#include "evita/li_util.h"

char16* lstrchrW(const char16* pwsz, char16 wch)
{
    while (0 != *pwsz)
    {
        if (wch == *pwsz)
        {
            return const_cast<char16*>(pwsz);
        } // if
        pwsz++;
    } // while
    return NULL;
} // lstrchrW


char16* lstrrchrW(const char16* pwsz, char16 wch)
{
    const char16* pFound = NULL;
    while (0 != *pwsz)
    {
        if (wch == *pwsz)
        {
            pFound = pwsz;
        } // if
        pwsz++;
    } // while
    return const_cast<char16*>(pFound);
} // lstrchrW
