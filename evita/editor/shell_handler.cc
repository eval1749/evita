// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/singleton_hwnd.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/shell_handler.h"

namespace editor {

ShellHandler::ShellHandler() {
}

ShellHandler::~ShellHandler() {
}

void ShellHandler::Start() {
  common::win::SingletonHwnd::instance()->AddObserver(this);
}

// common::win::SingletonHwnd::Observer
void ShellHandler::OnWndProc(HWND hwnd, UINT message, WPARAM,
                             LPARAM lParam) {
  switch (message) {
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
        p->m_hwnd = hwnd;
      ::SetEvent(g_hEvent);
      break;
    }

    case WM_COPYDATA: {
      auto const p = reinterpret_cast<COPYDATASTRUCT*>(lParam);
      auto const file_name = reinterpret_cast<base::char16*>(p->lpData);
      Application::instance()->view_event_handler()->OpenFile(
          dom::kInvalidWindowId, file_name);
      // TODO(yosi) Should we call |SetForegroundWindow|?
      break;
    }
  }
}

}  // namespace editor
