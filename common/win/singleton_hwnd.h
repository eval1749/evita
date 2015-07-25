// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_win_singleton_hwnd_h)
#define INCLUDE_common_win_singleton_hwnd_h

#include <memory>

#include "base/observer_list.h"
#include "common/common_export.h"
#include "common/memory/singleton.h"
#include "common/win/native_window.h"

namespace common {
namespace win {

class NativeWindow;

class COMMON_EXPORT SingletonHwnd :
    public MessageDelegate,
    public Singleton<SingletonHwnd> {
  DECLARE_SINGLETON_CLASS(SingletonHwnd);

  public: class COMMON_EXPORT Observer {
    protected: Observer();
    protected: virtual ~Observer();

    public: virtual void OnWndProc(HWND hwnd, UINT message, WPARAM wParam,
                                   LPARAM lParam) = 0;

    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  private: base::ObserverList<Observer> observers_;
  private: std::unique_ptr<NativeWindow> window_;

  private: SingletonHwnd();
  public: ~SingletonHwnd();

  public: void AddObserver(Observer* observer);
  public: void RemoveObserver(Observer* observer);

  // MessageDelegate
  private: virtual LRESULT WindowProc(UINT message, WPARAM wParam,
                                      LPARAM lParam) override;

  DISALLOW_COPY_AND_ASSIGN(SingletonHwnd);
};

} // namespace win
} // namespace common

#endif //!defined(INCLUDE_common_win_singleton_hwnd_h)
