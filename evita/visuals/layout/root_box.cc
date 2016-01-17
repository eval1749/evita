// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/root_box.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/layout/box_selection.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox(ViewLifecycle* lifecycle)
    : ContainerBox(this, &lifecycle->document()),
      lifecycle_(lifecycle),
      selection_(new BoxSelection()) {}

RootBox::~RootBox() {}

const Document& RootBox::document() const {
  return lifecycle_->document();
}

bool RootBox::InLayout() const {
  return lifecycle_->state() == ViewLifecycle::State::InLayout;
}

bool RootBox::InPaint() const {
  return lifecycle_->state() == ViewLifecycle::State::InPaint;
}

bool RootBox::IsLayoutClean() const {
  return lifecycle_->IsAtLeast(ViewLifecycle::State::LayoutClean);
}

bool RootBox::IsPaintClean() const {
  return lifecycle_->IsAtLeast(ViewLifecycle::State::PaintClean);
}

}  // namespace visuals
