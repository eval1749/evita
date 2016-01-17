// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/root_box.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_lifecycle.h"
#include "evita/visuals/layout/box_selection.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox(DocumentLifecycle* lifecycle)
    : ContainerBox(this, &lifecycle->document()),
      lifecycle_(lifecycle),
      selection_(new BoxSelection()) {}

RootBox::~RootBox() {}

const Document& RootBox::document() const {
  return lifecycle_->document();
}

bool RootBox::InLayout() const {
  return lifecycle_->state() == DocumentLifecycle::State::InLayout;
}

bool RootBox::InPaint() const {
  return lifecycle_->state() == DocumentLifecycle::State::InPaint;
}

bool RootBox::IsLayoutClean() const {
  return lifecycle_->IsAtLeast(DocumentLifecycle::State::LayoutClean);
}

bool RootBox::IsPaintClean() const {
  return lifecycle_->IsAtLeast(DocumentLifecycle::State::PaintClean);
}

}  // namespace visuals
