// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")

#include "evita/ui/compositor/compositor.h"

#include "base/memory/singleton.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/dx_device.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

Compositor::Compositor(gfx::DxDevice* device)
    : device_(device), need_commit_(false) {
  COM_VERIFY(::DCompositionCreateDevice2(device_->d2d_device(),
                                         IID_PPV_ARGS(&desktop_device_)));
#if _DEBUG
  common::ComPtr<IDCompositionDeviceDebug> debug_device;
  COM_VERIFY(debug_device.QueryFrom(desktop_device_));
  debug_device->EnableDebugCounters();
#endif
}

Compositor::Compositor() : Compositor(gfx::DxDevice::instance()) {}

Compositor::~Compositor() {}

// static
Compositor* Compositor::instance() {
  return GetInstance();
}

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

// static
Compositor* Compositor::GetInstance() {
  return base::Singleton<Compositor>().get();
}

void Compositor::WaitForCommitCompletion() {
  TRACE_EVENT0("ui", "ui::Compositor::WaitForCommitCompletion");
  COM_VERIFY(desktop_device_->WaitForCommitCompletion());
}

}  // namespace ui
