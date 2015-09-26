// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "common/win/singleton_hwnd.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace win {

//////////////////////////////////////////////////////////////////////
//
// SingletonHwnd::Observer
//
SingletonHwnd::Observer::Observer() {}

SingletonHwnd::Observer::~Observer() {}

//////////////////////////////////////////////////////////////////////
//
// SingletonHwnd
//
SingletonHwnd::SingletonHwnd() : window_(NativeWindow::Create(this)) {
  if (!base::MessageLoopForUI::IsCurrent()) {
    DLOG(ERROR) << "Cannot create window on non-UI thread.";
    return;
  }
  window_->CreateWindowEx(0, 0, L"SingletonHwnd", HWND_MESSAGE, Point(),
                          Size());
  DCHECK(window_->hwnd());
}

SingletonHwnd::~SingletonHwnd() {}

HWND SingletonHwnd::hwnd() const {
  DCHECK(window_->hwnd());
  return window_->hwnd();
}

HWND SingletonHwnd::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
  return hwnd();
}

void SingletonHwnd::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

// MessageDelegate
LRESULT SingletonHwnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnWndProc(*window_, message, wParam, lParam));
  return window_->DefWindowProc(message, wParam, lParam);
}

}  // namespace win
}  // namespace common
