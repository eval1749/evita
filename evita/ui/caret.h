// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_caret_h)
#define INCLUDE_evita_ui_caret_h

#include <memory>

#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_f.h"

namespace ui {

class Caret {
  private: gfx::RectF bounds_;
  private: base::Time last_blink_time_;
  private: bool visible_;

  protected: Caret();
  protected: virtual ~Caret();

  private: const gfx::RectF& bounds() const { return bounds_; }
  public: bool visible() const { return visible_; }

  public: void Blink(gfx::Canvas* canvas);
  public: void DidPaint(const gfx::RectF& paint_bounds);
  public: void Hide(gfx::Canvas* canvas);
  protected: virtual void Paint(gfx::Canvas* canvas,
                                const gfx::RectF& bounds) = 0;
  public: void Show(gfx::Canvas* canvas);
  public: void Update(gfx::Canvas* canvas,
                      const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_caret_h)
