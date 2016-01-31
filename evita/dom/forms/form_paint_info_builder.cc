// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_paint_info_builder.h"

namespace dom {

using Builder = FormPaintInfo::Builder;

//////////////////////////////////////////////////////////////////////
//
// Builder
//
Builder::Builder() {}
Builder::~Builder() {}

void Builder::set_caret_shape(domapi::CaretShape caret_shape) {
  paint_info_.caret_shape_ = caret_shape;
}

void Builder::set_hovered_control(FormControl* control) {
  paint_info_.hovered_control_ = control;
}

void Builder::set_interactive(bool value) {
  paint_info_.is_interactive_ = value;
}

}  // namespace dom
