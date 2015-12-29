// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "evita/visuals/demo/demo_application.h"

#if OS_WIN
// See "dome_window_win.cc" for |WinMain()| implementation.
#else
void main() {
  base::AtExitManager at_exit;
  visuals::DemoApplication().Run();
}
#endif
