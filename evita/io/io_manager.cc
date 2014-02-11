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

HANDLE g_hEvent;
bool g_fMultiple;

namespace {

// FinishSaveParam
struct FinishSaveParam {
  NewlineMode m_eNewline;
  FileTime m_ftLastWrite;
  uint m_nError;
  uint m_nFileAttrs;
  Buffer* m_pBuffer;
  char16 m_wszFileName[MAX_PATH];

  static void Run(LPARAM lParam) {
    reinterpret_cast<FinishSaveParam*>(lParam)->run();
  }

  void run() {
    if (!m_nError)
      m_pBuffer->SetFile(m_wszFileName, m_ftLastWrite);
    m_pBuffer->FinishIo(m_nError);
  }
};

//////////////////////////////////////////////////////////////////////
//
// IoManager
//
enum Message {
  Message_Start = WM_USER,

  Message_FinishSave,
};

}

IoManager::IoManager() {
}

IoManager::~IoManager() {
}

void IoManager::FinishSave(
    Buffer* pBuffer,
    const char16* pwszFileName,
    uint nError,
    NewlineMode eNewline,
    uint nFileAttrs,
    const FILETIME* pftLastWrite) {
  FinishSaveParam oParam;
  oParam.m_eNewline = eNewline;
  oParam.m_ftLastWrite = *pftLastWrite;
  oParam.m_nError = nError;
  oParam.m_nFileAttrs = nFileAttrs;
  oParam.m_pBuffer = pBuffer;

  ::lstrcpyW(oParam.m_wszFileName, pwszFileName);

  ::SendMessage(
      *Application::instance()->GetIoManager(),
      Message_FinishSave,
      0,
      reinterpret_cast<LPARAM>(&oParam));
}

LRESULT IoManager::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CREATE: {
      if (g_fMultiple)
        break;
      auto const hMapping = ::CreateFileMapping(
          INVALID_HANDLE_VALUE, // hFile
          nullptr, // lpAttributes
          PAGE_READWRITE, // flProtect
          0, // dwMaximumSizeHigh
          k_cbFileMapping, // dwMaximumSizeLow
          k_wszFileMapping); // lpName

      DCHECK(hMapping);

      auto const p = reinterpret_cast<SharedArea*>(::MapViewOfFile(
          hMapping,
          FILE_MAP_READ | FILE_MAP_WRITE,
          0, // dwFileOffsetHigh
          0, // dwFileOffsetLow
          k_cbFileMapping));

      if (p)
        p->m_hwnd = *this;
      ::SetEvent(g_hEvent);
      break;
    }

    case WM_COPYDATA: {
      auto const p = reinterpret_cast<COPYDATASTRUCT*>(lParam);
      auto const filename = reinterpret_cast<base::char16*>(p->lpData);
      Application::instance()->view_event_handler()->OpenFile(
          views::kInvalidWindowId, filename);
      // TODO(yosi) Should we call |SetForegroundWindow|?
      return true;
    }

    case Message_FinishSave:
      FinishSaveParam::Run(lParam);
      return 0;
  }
  return NativeWindow::WindowProc(uMsg, wParam, lParam);
}

void IoManager::Realize() {
  CreateWindowEx(0, 0, L"IoManager", HWND_MESSAGE, common::win::Point(),
                 common::win::Size());
  DCHECK(*this);
}
