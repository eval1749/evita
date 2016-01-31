// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_paint_info.h"

#include "evita/dom/public/caret_shape.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormPaintInfo
//
FormPaintInfo::FormPaintInfo() : caret_shape_(domapi::CaretShape::None) {}
FormPaintInfo::~FormPaintInfo() {}

}  // namespace dom
