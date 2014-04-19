// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_caret_h)
#define INCLUDE_evita_ui_caret_h

#include <memory>

#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "evita/gfx/graphics.h"
#include "evita/gfx/rect_f.h"

namespace ui {

// Represents caret in per-thread queue. To blink caret, we must track
// caret size. If we call CreateCaret, caret doesn't blink.
// Note: Caret should be lived until timer is fired.
class Caret final : public common::Singleton<Caret> {
  DECLARE_SINGLETON_CLASS(Caret);

  public: class Owner {
    public: Owner();
    public: virtual ~Owner();

    public: virtual void UpdateCaret(gfx::Graphics* gfx) = 0;
  };

  private: base::Time last_blink_time_;
  private: gfx::RectF bounds_;
  private: Owner* owner_;
  private: bool shown_;

  public: Caret();
  public: ~Caret();

  public: void Blink(gfx::Graphics* gfx);
  public: void Give(Owner* owner);
  public: void Take(Owner* owner);
  public: void Update(gfx::Graphics* gfx, const gfx::RectF& rect);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_caret_h)
