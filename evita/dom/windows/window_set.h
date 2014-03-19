// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_windows_window_set_h)
#define INCLUDE_evita_dom_windows_window_set_h

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/windows/window_id.h"
#include "evita/gc/weak_ptr.h"

namespace dom {

class Window;

class WindowSet : public common::Singleton<WindowSet> {
  DECLARE_SINGLETON_CLASS(WindowSet);

  private: typedef WindowId WindowId;

  private: std::unordered_map<WindowId, gc::WeakPtr<Window>> map_;

  private: WindowSet();
  public: ~WindowSet();

  public: void DidDestroyWidget(WindowId window_id);
  public: Window* Find(WindowId window_id) const;
  public: void Register(Window* window);
  public: void ResetForTesting();
  public: void Unregister(WindowId window_id);

  DISALLOW_COPY_AND_ASSIGN(WindowSet);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_windows_window_set_h)
