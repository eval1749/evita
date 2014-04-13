// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/point.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Point
//
Point::Point(float x, float y) : x_(x), y_(y) {
}

Point::Point() {
}

Point::~Point() {
}

// static
Point* Point::NewPoint(v8_glue::Optional<float> opt_x,
                       v8_glue::Optional<float> opt_y) {
  return new Point(opt_x.get(0), opt_y.get(0));
}

}  // namespace dom
