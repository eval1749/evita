// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
// L1 C4917 'declarator' : a GUID can only be associated with a class, interface
// or namespace
#pragma warning(push)
#pragma warning(disable : 4917)
#include <dcomp.h>
#pragma warning(pop)
#pragma comment(lib, "dcomp.lib")

#include "evita/ui/compositor/compositor.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx/dx_device.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

Compositor::Compositor() : need_commit_(false) {
  COM_VERIFY(::DCompositionCreateDevice2(
      gfx::DxDevice::instance()->d2d_device(), IID_PPV_ARGS(&desktop_device_)));

#if _DEBUG
  common::ComPtr<IDCompositionDeviceDebug> debug_device;
  COM_VERIFY(debug_device.QueryFrom(desktop_device_));
  debug_device->EnableDebugCounters();
#endif
}

Compositor::~Compositor() {}

void Compositor::CommitIfNeeded() {
  if (!need_commit_)
    return;
  TRACE_EVENT0("ui", "ui::Compositor::CommitIfNeeded");
#if _DEBUG
  common::ComPtr<IDCompositionDevice> device;
  BOOL is_valid;
  COM_VERIFY(device.QueryFrom(desktop_device_));
  COM_VERIFY(device->CheckDeviceState(&is_valid));
  DCHECK(is_valid);
#endif
  COM_VERIFY(desktop_device_->Commit());
  need_commit_ = false;
}

common::ComPtr<IDCompositionVisual2> Compositor::CreateVisual() {
  common::ComPtr<IDCompositionVisual2> visual;
  COM_VERIFY(desktop_device_->CreateVisual(&visual));
  return visual;
}

void Compositor::WaitForCommitCompletion() {
  TRACE_EVENT0("ui", "ui::Compositor::WaitForCommitCompletion");
  COM_VERIFY(desktop_device_->WaitForCommitCompletion());
}

}  // namespace ui
