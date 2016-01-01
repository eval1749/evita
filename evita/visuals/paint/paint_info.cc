// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/paint/paint_info.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// PaintInfo
//
PaintInfo::PaintInfo(const FloatRect& cull_rect) : cull_rect_(cull_rect) {}
PaintInfo::~PaintInfo() {}

}  // namespace visuals
