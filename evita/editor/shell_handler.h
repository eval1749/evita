// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_editor_shell_handler_h)
#define INCLUDE_evita_editor_shell_handler_h

#include "common/win/singleton_hwnd.h"

namespace editor {

class ShellHandler : public common::Singleton<ShellHandler>,
                     public common::win::SingletonHwnd::Observer {
  DECLARE_SINGLETON_CLASS(ShellHandler);

  private: ShellHandler();
  public: virtual ~ShellHandler();

  public: void Start();

  // common::win::SingletonHwnd::Observer
  private: virtual void OnWndProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam) override;

  DISALLOW_COPY_AND_ASSIGN(ShellHandler);
};

}  // namespace editor

//////////////////////////////////////////////////////////////////////
//
// SharedArea
//
struct SharedArea {
  HWND m_hwnd;
  char16 m_wsz[1];
}; // SharedArea

const char16 k_wszFileMapping[] =
    L"Local\\03002DEC-D63E-4551-9AE8-B88E8C586376";

const DWORD k_cbFileMapping = 1024 * 64;

extern HANDLE g_hEvent;
extern bool g_fMultiple;

#endif //!defined(INCLUDE_evita_editor_shell_handler_h)
