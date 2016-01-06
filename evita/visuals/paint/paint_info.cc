// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/paint/paint_info.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// PaintInfo
//
PaintInfo::PaintInfo(const FloatRect& cull_rect,
                     const base::string16& debug_text)
    : cull_rect_(cull_rect), debug_text_(debug_text) {}

PaintInfo::PaintInfo(const FloatRect& cull_rect)
    : PaintInfo(cull_rect, base::string16()) {}

PaintInfo::~PaintInfo() {}

}  // namespace visuals
