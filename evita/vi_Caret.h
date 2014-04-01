//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - visual - text pane
// listener/winapp/vi_text_pane.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_TextEditWindow.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_caret_h)
#define INCLUDE_listener_winapp_visual_caret_h

#include "base/time/time.h"
#include "evita/gfx/rect_f.h"
#include "evita/li_util.h"
#include <memory>

namespace gfx {
class Grapchis;
}

// Represents caret in per-thread queue. To blink caret, we must track
// caret size. If we call CreateCaret, caret doesn't blink.
// Note: Caret should be lived until timer is fired.
class Caret {
  private: class BackingStore;

  private: const std::unique_ptr<BackingStore> backing_store_;
  private: base::Time last_blink_time_;
  private: const gfx::Graphics* gfx_;
  private: gfx::RectF rect_;
  private: bool shown_;
  private: bool should_blink_;
  private: bool taken_;

  public: Caret();
  public: ~Caret();
  public: void Blink();
  public: void Hide();
  public: void Give();
  public: void Reset();
  private: void Show();
  public: void ShouldBlink() { should_blink_ = true; }
  // TODO: We should pass Widget to Caret::Take() instead of gfx::Graphics.
  public: void Take(const gfx::Graphics& gfx);
  public: void Update(const gfx::RectF& rect);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

#endif //!defined(INCLUDE_listener_winapp_visual_caret_h)
