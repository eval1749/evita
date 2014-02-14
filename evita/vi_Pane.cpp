// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_Pane.h"

#include "common/win/native_window.h"
#include "evita/vi_Frame.h"

Pane::Pane(std::unique_ptr<common::win::NativeWindow>&& native_window)
    : CommandWindow(std::move(native_window)),
      m_pwszName(L"") {
}

Pane::Pane()
    : Pane(std::move(std::unique_ptr<common::win::NativeWindow>())) {
}

Pane::~Pane() {
}

void Pane::Activate() {
  GetFrame()->DidActivatePane(this);
}

HCURSOR Pane::GetCursorAt(const common::win::Point&) const {
  return nullptr;
}
