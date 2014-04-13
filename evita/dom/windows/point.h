// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_point_h)
#define INCLUDE_evita_dom_point_h

#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace base {
class Point;
}

namespace dom {

// TODO(yosi): We should move |x| and |y| properties to JavaScript.
class Point : public v8_glue::Scriptable<Point> {
  DECLARE_SCRIPTABLE_OBJECT(Point);

  private: float x_;
  private: float y_;

  public: Point(float x, float y);
  public: Point();
  public: virtual ~Point();

  public: float x() const { return x_; }
  public: void set_x(float x) { x_ = x; }
  public: float y() const { return y_; }
  public: void set_y(float y) { y_ = y; }

  public: static Point* NewPoint(v8_glue::Optional<float> opt_x,
                                 v8_glue::Optional<float> opt_y);

  DISALLOW_COPY_AND_ASSIGN(Point);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_point_h)
