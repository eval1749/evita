// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_COMPONENTS_WIN_REGISTRY_WIN_REGISTRY_H_
#define EVITA_DOM_COMPONENTS_WIN_REGISTRY_WIN_REGISTRY_H_

#include "evita/ginx/scriptable.h"

#include "base/macros.h"

namespace dom {

class ExceptionState;

namespace bindings {
class WinRegistryClass;
}

//////////////////////////////////////////////////////////////////////
//
// WinRegistry
//
class WinRegistry final : public ginx::Scriptable<WinRegistry> {
  DECLARE_SCRIPTABLE_OBJECT(WinRegistry);

 public:
  ~WinRegistry() final;

 private:
  friend class bindings::WinRegistryClass;

  WinRegistry();

  static base::string16 ReadString(const base::string16& path,
                                   ExceptionState* exception_state);

  DISALLOW_COPY_AND_ASSIGN(WinRegistry);
};

}  // namespace dom

#endif  // EVITA_DOM_COMPONENTS_WIN_REGISTRY_WIN_REGISTRY_H_
