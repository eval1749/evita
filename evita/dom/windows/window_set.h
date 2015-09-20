// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_WINDOW_SET_H_
#define EVITA_DOM_WINDOWS_WINDOW_SET_H_

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/windows/window_id.h"
#include "evita/gc/weak_ptr.h"

namespace dom {

class Window;

class WindowSet final : public common::Singleton<WindowSet> {
  DECLARE_SINGLETON_CLASS(WindowSet);

 public:
  ~WindowSet() final;

  void DidDestroyWidget(WindowId window_id);
  Window* Find(WindowId window_id) const;
  void Register(Window* window);
  void ResetForTesting();
  void Unregister(WindowId window_id);

 private:
  WindowSet();

  std::unordered_map<WindowId, gc::WeakPtr<Window>> map_;

  DISALLOW_COPY_AND_ASSIGN(WindowSet);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_WINDOW_SET_H_
