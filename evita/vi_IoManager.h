//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_gateway.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_IoManager.h#1 $
//
#if !defined(INCLUDE_visual_IoManager_h)
#define INCLUDE_visual_IoManager_h

#include "base/win/native_window.h"

class Buffer;

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
class IoManager : public base::win::NativeWindow
{
    public: IoManager();

    // [F]
    public: static void FinishLoad(
        Buffer*,
        const char16*,
        uint,
        NewlineMode,
        uint,
        const FILETIME* );

    public: static void FinishSave(
        Buffer*,
        const char16*,
        uint,
        NewlineMode,
        uint,
        const FILETIME* );

    // [I]
    public: static void InsertString(
        Buffer*, 
        Posn, 
        const char16*, 
        Count );

    // [R]
    public: void Realize();

    // [V]
    private: void visitFile(const char16*);

    // [W]
    protected: virtual LRESULT WindowProc(UINT, WPARAM, LPARAM) override;

    DISALLOW_COPY_AND_ASSIGN(IoManager);
}; // IoManager


//////////////////////////////////////////////////////////////////////
//
// SharedArea
//
struct SharedArea
{
    HWND    m_hwnd;
    char16  m_wsz[1];
}; // SharedArea



const char16 k_wszFileMapping[] =
    L"Local\\03002DEC-D63E-4551-9AE8-B88E8C586376";

const uint k_cbFileMapping = 1024 * 64;

extern HANDLE g_hEvent;
extern bool   g_fMultiple;

#endif //!defined(INCLUDE_visual_IoManager_h)
