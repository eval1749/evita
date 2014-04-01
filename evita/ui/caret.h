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
class Caret : public common::Singleton<Caret>,
              public gfx::Graphics::Observer {
  DECLARE_SINGLETON_CLASS(Caret);

  private: class BackingStore;

  private: std::unique_ptr<BackingStore> backing_store_;
  private: base::Time last_blink_time_;
  private: const gfx::Graphics* gfx_;
  private: gfx::RectF rect_;
  private: bool shown_;
  private: bool stop_blinking_;
  private: bool taken_;

  public: Caret();
  public: ~Caret();
  public: void Blink(const gfx::Graphics* gfx);
  public: void Hide(const gfx::Graphics* gfx);
  public: void Hide();
  public: void Give(const gfx::Graphics* gfx);
  private: void Show(const gfx::Graphics* gfx);
  public: void StartBlinking();
  public: void StopBlinking();
  // TODO: We should pass Widget to Caret::Take() instead of gfx::Graphics.
  public: void Take(const gfx::Graphics* gfx);
  public: void Update(const gfx::Graphics* gfx, const gfx::RectF& rect);

  // gfx::Graphics::Observer
  private: void ShouldDiscardResources() override;

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_caret_h)
