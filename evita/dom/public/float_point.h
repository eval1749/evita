// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_float_point_h)
#define INCLUDE_evita_dom_public_float_point_h

namespace domapi {

class FloatPoint {
  private: float x_;
  private: float y_;

  public: FloatPoint(float x, float y);
  public: FloatPoint();
  public: ~FloatPoint();

  public: float x() const { return x_; }
  public: float y() const { return y_; }
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_float_point_h)
