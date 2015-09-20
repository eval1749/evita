// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FLOAT_RECT_H_
#define EVITA_DOM_PUBLIC_FLOAT_RECT_H_

namespace domapi {

class FloatPoint;

class FloatRect final {
 public:
  FloatRect(float x, float y, float width, float height);
  FloatRect();
  ~FloatRect();

  float bottom() const { return y_ + height_; }
  float height() const { return height_; }
  float left() const { return x_; }
  FloatPoint origin() const;
  float right() const { return x_ + width_; }
  FloatPoint bottom_right() const;
  float top() const { return x_; }
  float width() const { return width_; }
  float x() const { return x_; }
  float y() const { return y_; }

 private:
  float x_;
  float y_;
  float width_;
  float height_;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FLOAT_RECT_H_
