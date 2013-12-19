//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/vi_defs.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_defs.h#1 $
//
#if !defined(INCLUDE_listener_winapp_visual_defs_h)
#define INCLUDE_listener_winapp_visual_defs_h

#include "./resource.h"
#include "gfx/rect.h"

// TODO: We should get rid of |using gfx::Point|.
using gfx::Point;
// TODO: We should get rid of |using gfx::Size|.
using gfx::Size;
// TODO: We should get rid of |using gfx::Rect|.
using gfx::Rect;

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;

#endif //!defined(INCLUDE_listener_winapp_visual_defs_h)
