// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/root_box.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/layout/box_selection.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox(const Document& document) : ContainerBox(this, &document) {}
RootBox::~RootBox() {}

bool RootBox::InLayout() const {
  return lifecycle_.state() == BoxTreeLifecycle::State::InLayout;
}

bool RootBox::InPaint() const {
  return lifecycle_.state() == BoxTreeLifecycle::State::InPaint;
}

bool RootBox::IsLayoutClean() const {
  return lifecycle_.IsAtLeast(BoxTreeLifecycle::State::LayoutClean);
}

bool RootBox::IsPaintClean() const {
  return lifecycle_.IsAtLeast(BoxTreeLifecycle::State::PaintClean);
}

}  // namespace visuals
