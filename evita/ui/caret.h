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

// Represents caret in per-thread queue. To blink caret, we must track
// caret size. If we call CreateCaret, caret doesn't blink.
// Note: Caret should be lived until timer is fired.
class Caret final : public common::Singleton<Caret> {
  DECLARE_SINGLETON_CLASS(Caret);

  public: class Delegate {
    public: Delegate();
    public: virtual ~Delegate();

    public: virtual void HideCaret(gfx::Canvas* canvas,
                                   const Caret& caret) = 0;
    public: virtual void PaintCaret(gfx::Canvas* canvas,
                                    const Caret& caret) = 0;
    public: virtual void ShowCaret(gfx::Canvas* canvas,
                                   const Caret& caret) = 0;
  };

  private: gfx::RectF bounds_;
  private: Delegate* owner_;
  private: base::Time last_blink_time_;
  private: bool shown_;

  public: Caret();
  public: ~Caret();

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: bool visible() const { return shown_; }
  public: Delegate* owner() const { return owner_; }

  public: void Blink(Delegate* delegate, gfx::Canvas* canvas);
  // TODO(eval1749) We should use single parameter version of |Blink|. It is
  // used for blinking caret in |FormWindow|. We should use |ui::Animator| for
  // blinking caret for |FormWindow|.
  public: void Blink(gfx::Canvas* canvas);
  public: void DidPaint(Delegate* delegate, const gfx::RectF& paint_bounds);
  public: void Give(Delegate* delegate);
  public: void Hide(Delegate* delegate);
  public: void Take(Delegate* delegate);
  public: void Update(Delegate* delegate, gfx::Canvas* canvas,
                      const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_caret_h)
