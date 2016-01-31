// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_paint_info.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormPaintInfo
//
FormPaintInfo::FormPaintInfo(gfx::Canvas* canvas,
                             const gfx::RectF& bounds,
                             FormPaintState* paint_state)
    : bounds_(bounds), canvas_(canvas), paint_state_(paint_state) {}

FormPaintInfo::~FormPaintInfo() {}

}  // namespace views
