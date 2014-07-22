// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/compositor.h"

// L1 C4917 'declarator' : a GUID can only be associated with a class, interface
// or namespace
#pragma warning(disable: 4917)
#include <dcomp.h>

#include "evita/gfx_base.h"

#pragma comment(lib, "dcomp.lib")

namespace ui {

Compositor::Compositor(ID2D1Device* d2d_device) : need_commit_(false) {
  COM_VERIFY(::DCompositionCreateDevice2(
      d2d_device,
      IID_PPV_ARGS(&composition_device_)));
}

Compositor::~Compositor() {
}

void Compositor::Commit() {
  if (!need_commit_)
    return;
  COM_VERIFY(composition_device_->Commit());
  need_commit_ = false;
}

common::ComPtr<IDCompositionVisual2> Compositor::CreateVisual() {
  common::ComPtr<IDCompositionVisual2> visual;
  COM_VERIFY(composition_device_->CreateVisual(&visual));
  return visual;
}

}  // namespace ui
