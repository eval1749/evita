// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/float_point.h"

namespace domapi {

FloatPoint::FloatPoint(float x, float y) : x_(x), y_(y) {}

FloatPoint::FloatPoint() : FloatPoint(0.0f, 0.0f) {}

FloatPoint::~FloatPoint() {}

}  // namespace domapi
