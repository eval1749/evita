// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "common/win/native_window.h"
#include "evita/editor/application_proxy.h"

extern HINSTANCE   g_hInstance;
extern HINSTANCE   g_hResource;

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
  g_hInstance = hInstance;
  g_hResource = hInstance;

  return editor::ApplicationProxy::instance()->Run();
}
