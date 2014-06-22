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
class Canvas;
}

namespace views {
class Window;
}

// Represents caret in per-thread queue. To blink caret, we must track
// caret size. If we call CreateCaret, caret doesn't blink.
// Note: Caret should be lived until timer is fired.
class Caret {
  public: class HideScope {
    private: gfx::Canvas* const canvas_;
    private: Caret* const caret_;
    private: bool const shown_;

    public: HideScope(Caret* caret, gfx::Canvas* gfx);
    public: ~HideScope();

    DISALLOW_COPY_AND_ASSIGN(HideScope);
  };
  friend class HideScope;

  public: class Updater {
    private: Caret* const caret_;

    public: Updater(Caret* caret);
    public: ~Updater();

    public: void Clear();
    public: void Update(const gfx::Canvas* gfx, const gfx::RectF& rect);

    DISALLOW_COPY_AND_ASSIGN(Updater);
  };
  friend class Updater;

  private: base::Time last_blink_time_;
  private: views::Window* owner_;
  private: gfx::RectF bounds_;
  private: bool shown_;

  public: Caret();
  public: ~Caret();
  public: void Blink(const gfx::Canvas* gfx);
  public: void Give(views::Window* window, const gfx::Canvas* gfx);
  public: void Hide(const gfx::Canvas* gfx_);
  private: void Show(const gfx::Canvas* gfx);
  public: void Take(views::Window* owner);
  private: void Update(const gfx::Canvas* gfx, const gfx::RectF& rect);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

#endif //!defined(INCLUDE_listener_winapp_visual_caret_h)
