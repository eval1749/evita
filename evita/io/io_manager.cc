// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_manager.h"

#pragma warning(push)
#pragma warning(disable: 4365)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/dom/buffer.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/views/window.h"
#include "evita/views/window_set.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

HANDLE g_hEvent;
bool   g_fMultiple;

namespace
{

// FinishLoadParam
struct FinishLoadParam
{
    dom::ViewDelegate::LoadFileCallback callback_;
    base::string16 file_name_;
    NewlineMode     m_eNewline;
    FileTime        m_ftLastWrite;
    uint            m_nError;
    uint            m_nFileAttrs;
    Buffer*         m_pBuffer;

    static void Run(LPARAM lParam) {
      reinterpret_cast<FinishLoadParam*>(lParam)->run();
    }

    void run() {
        if (ERROR_HANDLE_EOF == m_nError)
        {
            m_pBuffer->SetFile(file_name_.c_str(), m_ftLastWrite);
            if (NewlineMode_Detect == m_pBuffer->GetNewline())
            {
                m_pBuffer->SetNewline(m_eNewline);
            }

            if (m_nFileAttrs & FILE_ATTRIBUTE_READONLY)
            {
                m_pBuffer->SetReadOnly(true);
            }

            // Make EOF not error
            m_nError = 0;
        } // if

        if (!m_nError)
        {
            for (auto window : views::Window::all_windows()) {
                if (auto text_window = window->as<TextEditWindow>())
                    text_window->GetSelection()->RestoreForReload();
            }
        }

        m_pBuffer->FinishIo(m_nError);
        Application::instance()->view_event_handler()->RunCallback(
            base::Bind(callback_, m_nError));
    } // run
}; // FinishLoadParam


// FinishSaveParam
struct FinishSaveParam
{
    NewlineMode     m_eNewline;
    FileTime        m_ftLastWrite;
    uint            m_nError;
    uint            m_nFileAttrs;
    Buffer*         m_pBuffer;
    char16          m_wszFileName[MAX_PATH];

    static void Run(LPARAM lParam)
    {
        reinterpret_cast<FinishSaveParam*>(lParam)->run();
    } // Run

    void run()
    {
        if (0 == m_nError)
        {
            m_pBuffer->SetFile(m_wszFileName, m_ftLastWrite);
        } // if

        m_pBuffer->FinishIo(m_nError);
    } // run
}; // FinishSaveParam


//////////////////////////////////////////////////////////////////////
//
// InsertStringParam
//
struct InsertStringParam
{
    Count           m_cwch;
    Posn            m_lPosn;
    text::Buffer*   m_pBuffer;
    const char16*   m_pwch;

    static void Run(LPARAM lParam)
    {
        reinterpret_cast<InsertStringParam*>(lParam)->run();
    } // Run

    void run()
    {
        m_pBuffer->InternalInsert(
            m_lPosn,
            m_pwch,
            m_cwch );
    } // Message_InsertString
}; // InsertStringParam

enum Message
{
    Message_Start = WM_USER,

    Message_FinishLoad,
    Message_FinishSave,
    Message_InsertString,
}; // Message

} // namespace

IoManager::IoManager() {
}

//////////////////////////////////////////////////////////////////////
//
// IoManager::FinishLoad
//
void
IoManager::FinishLoad(
    const dom::ViewDelegate::LoadFileCallback& callback,
    Buffer*             pBuffer,
    const base::string16& file_name,
    uint                nError,
    NewlineMode         eNewline,
    uint                nFileAttrs,
    const FILETIME*     pftLastWrite )
{
    FinishLoadParam oParam;
    oParam.callback_ = callback;
    oParam.file_name_ = file_name;
    oParam.m_eNewline     = eNewline;
    oParam.m_ftLastWrite  = *pftLastWrite;
    oParam.m_nError       = nError;
    oParam.m_nFileAttrs   = nFileAttrs;
    oParam.m_pBuffer      = pBuffer;

    ::SendMessage(
        *Application::instance()->GetIoManager(),
        Message_FinishLoad,
        0,
        reinterpret_cast<LPARAM>(&oParam) );
} // IoManager::FinishLoad


//////////////////////////////////////////////////////////////////////
//
// IoManager::FinishSave
//
void
IoManager::FinishSave(
    Buffer*             pBuffer,
    const char16*       pwszFileName,
    uint                nError,
    NewlineMode         eNewline,
    uint                nFileAttrs,
    const FILETIME*     pftLastWrite )
{
    FinishSaveParam oParam;
    oParam.m_eNewline     = eNewline;
    oParam.m_ftLastWrite  = *pftLastWrite;
    oParam.m_nError       = nError;
    oParam.m_nFileAttrs   = nFileAttrs;
    oParam.m_pBuffer      = pBuffer;

    ::lstrcpyW(oParam.m_wszFileName, pwszFileName);

    ::SendMessage(
        *Application::instance()->GetIoManager(),
        Message_FinishSave,
        0,
        reinterpret_cast<LPARAM>(&oParam) );
} // IoManager::FinishSave


//////////////////////////////////////////////////////////////////////
//
// IoManager::InsertString
//
void
IoManager::InsertString(
    Buffer*         pBuffer,
    Posn            lPosn,
    const char16*   pwch,
    Count           cwch )
{
    InsertStringParam oParam;
    oParam.m_cwch    = cwch;
    oParam.m_lPosn   = lPosn;
    oParam.m_pBuffer = pBuffer;
    oParam.m_pwch    = pwch;

    ::SendMessage(
        *Application::instance()->GetIoManager(),
        Message_InsertString,
        0,
        reinterpret_cast<LPARAM>(&oParam) );
} // IoManager::InsertString


//////////////////////////////////////////////////////////////////////
//
// IoManager::WindowProc
//
LRESULT
IoManager::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        when (g_fMultiple) break;

        HANDLE hMapping = ::CreateFileMapping(
            INVALID_HANDLE_VALUE,   // hFile
            NULL,                   // lpAttributes
            PAGE_READWRITE,         // flProtect
            0,                      //  dwMaximumSizeHigh
            k_cbFileMapping,        // dwMaximumSizeLow
            k_wszFileMapping );     // lpName
            
        ASSERT(NULL != hMapping);

        SharedArea* p = reinterpret_cast<SharedArea*>(::MapViewOfFile(
            hMapping,
            FILE_MAP_READ | FILE_MAP_WRITE,
            0,      // dwFileOffsetHigh
            0,      // dwFileOffsetLow
            k_cbFileMapping ) );

        if (NULL != p)
        {
            p->m_hwnd = *this;
        }

        ::SetEvent(g_hEvent);
        break;
    } // WM_CREATE

    case WM_COPYDATA:
    {
        COPYDATASTRUCT* p = reinterpret_cast<COPYDATASTRUCT*>(lParam);
        auto const filename = reinterpret_cast<base::char16*>(p->lpData);
        Application::instance()->view_event_handler()->OpenFile(
            views::kInvalidWindowId, filename);
        // TODO(yosi) Should we call |SetForegroundWindow|?
        return true;
    } // WM_COPYDATA

    case Message_FinishLoad:
        FinishLoadParam::Run(lParam);
        return 0;

    case Message_FinishSave:
        FinishSaveParam::Run(lParam);
        return 0;

    case Message_InsertString:
        InsertStringParam::Run(lParam);
        return 0;
    } // switch message

    return NativeWindow::WindowProc(uMsg, wParam, lParam);
} // IoManager::onMessage


//////////////////////////////////////////////////////////////////////
//
// IoManager::Realize
//
void IoManager::Realize()
{
    CreateWindowEx(0, 0, L"IoManager", HWND_MESSAGE, Point(), Size());
    ASSERT(NULL != *this);
} // IoManager::Realize
