// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_manager_h)
#define INCLUDE_evita_io_io_manager_h

#include "common/win/native_window.h"
#include "evita/dom/view_delegate.h"

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
class IoManager : public common::win::NativeWindow
{
    public: IoManager();

    // [F]
    public: static void FinishLoad(
        const dom::ViewDelegate::LoadFileCallback& callback,
        Buffer* buffer,
        const base::string16& file_name,
        uint32_t error_code,
        NewlineMode newline_mode,
        uint32_t file_attributes,
        const FILETIME* last_write_time);

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

#endif //!defined(INCLUDE_evita_io_io_manager_h)
