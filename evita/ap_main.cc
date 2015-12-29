// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <commctrl.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "common/win/com_init.h"
#include "common/win/native_window.h"
#include "evita/editor/application_proxy.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;

namespace {

void InitWindowsCommonControls() {
  INITCOMMONCONTROLSEX init_params;
  init_params.dwSize = sizeof(init_params);
  init_params.dwICC = ICC_BAR_CLASSES;
  if (::InitCommonControlsEx(&init_params))
    return;
  ::FatalAppExit(0, L"Failed to call InitCommonControlsEx()");
  NOTREACHED();
}

}  // namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  base::AtExitManager at_exit;
  base::i18n::InitializeICU();
  base::CommandLine::set_slash_is_not_a_switch();
  base::CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }

  common::win::NativeWindow::Init(hInstance);
  common::win::ComInit com_init;
  InitWindowsCommonControls();
  g_hInstance = hInstance;
  g_hResource = hInstance;

  return editor::ApplicationProxy::instance()->Run();
}
