// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CARET_H_
#define EVITA_UI_CARET_H_

#include <memory>

#include "base/time/time.h"
#pragma warning(push)
#pragma warning(disable : 4625 4626)
#include "base/timer/timer.h"
#pragma warning(pop)
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace ui {

class CaretOwner;

//////////////////////////////////////////////////////////////////////
//
// Caret
//
class Caret {
 public:
  virtual ~Caret();

  bool visible() const { return visible_; }

  void Blink(gfx::Canvas* canvas, base::Time time);
  void DidPaint(const gfx::RectF& paint_bounds);
  void Hide(gfx::Canvas* canvas);
  void Reset();
  void Update(gfx::Canvas* canvas,
              base::Time time,
              const gfx::RectF& new_bounds);

 protected:
  explicit Caret(CaretOwner* owner);

  virtual void DidChangeCaret();
  virtual void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) = 0;

 private:
  const gfx::RectF& bounds() const { return bounds_; }

  void DidFireTimer();

  base::TimeDelta blink_interval_;
  gfx::RectF bounds_;
  base::Time last_blink_time_;
  CaretOwner* const owner_;
  base::RepeatingTimer timer_;
  bool visible_;

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

}  // namespace ui

#endif  // EVITA_UI_CARET_H_
