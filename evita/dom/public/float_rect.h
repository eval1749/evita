// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_float_rect_h)
#define INCLUDE_evita_dom_public_float_rect_h

namespace domapi {

class FloatPoint;

class FloatRect {
  private: float x_;
  private: float y_;
  private: float width_;
  private: float height_;

  public: FloatRect(float x, float y, float width, float height);
  public: FloatRect();
  public: ~FloatRect();

  public: float bottom() const { return y_ + height_; }
  public: float height() const { return height_; }
  public: float left() const { return x_; }
  public: FloatPoint origin() const;
  public: float right() const { return x_ + width_; }
  public: FloatPoint right_bottom() const;
  public: float top() const { return x_; }
  public: float width() const { return width_; }
  public: float x() const { return x_; }
  public: float y() const { return y_; }
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_float_rect_h)
