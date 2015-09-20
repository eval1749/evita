// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FLOAT_POINT_H_
#define EVITA_DOM_PUBLIC_FLOAT_POINT_H_

namespace domapi {

class FloatPoint final {
 public:
  FloatPoint(float x, float y);
  FloatPoint();
  ~FloatPoint();

  float x() const { return x_; }
  float y() const { return y_; }

 private:
  float x_;
  float y_;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FLOAT_POINT_H_
