// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_WIN_SINGLETON_HWND_H_
#define COMMON_WIN_SINGLETON_HWND_H_

#include <memory>

#include "base/observer_list.h"
#include "common/common_export.h"
#include "common/memory/singleton.h"
#include "common/win/native_window.h"

namespace common {
namespace win {

class NativeWindow;

class COMMON_EXPORT SingletonHwnd : public MessageDelegate,
                                    public Singleton<SingletonHwnd> {
  DECLARE_SINGLETON_CLASS(SingletonHwnd);

 public:
  class COMMON_EXPORT Observer {
   public:
    virtual void OnWndProc(HWND hwnd,
                           UINT message,
                           WPARAM wParam,
                           LPARAM lParam) = 0;

   protected:
    Observer();
    virtual ~Observer();

   private:
    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  ~SingletonHwnd();

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // MessageDelegate
 private:
  SingletonHwnd();

  LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

  base::ObserverList<Observer> observers_;
  std::unique_ptr<NativeWindow> window_;

  DISALLOW_COPY_AND_ASSIGN(SingletonHwnd);
};

}  // namespace win
}  // namespace common

#endif  // COMMON_WIN_SINGLETON_HWND_H_
