// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/label_control.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// LabelControl
//
LabelControl::LabelControl(const base::string16& text) : text_(text) {}

LabelControl::~LabelControl() {}

void LabelControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  NotifyControlChange();
}

}  // namespace dom
