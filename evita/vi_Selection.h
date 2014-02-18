// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_listener_winapp_visual_selection_h)
#define INCLUDE_listener_winapp_visual_selection_h

#include "base/strings/string16.h"
#include "evita/text/range.h"
#include "evita/ed_Style.h"
#include "evita/gfx_base.h"

class TextEditWindow;

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
// m_lRestoreLineNum
// A line number which selection belongs before reloading. We try to
// move selection to this line after reloading.
//
class Selection : public text::Range {
  private: typedef text::Range Range;

  private: Color m_crColor;
  private: Color m_crBackground;
  private: bool m_fStartIsActive;
  private: Buffer* m_pBuffer;
  private: TextEditWindow* m_pWindow;

  private : explicit Selection(const text::Range& range);
  public: ~Selection();

  public: void set_window(TextEditWindow* window) {
    m_pWindow = window;
  }

  // [C]
  public: void Collapse(CollapseWhich = Collapse_Start);
  public: static Selection* Create(const text::Range& range);

  // [G]
  public: Color GetBackground() const { return m_crBackground; }
  public: Buffer* GetBuffer() const { return m_pBuffer; }
  public: Color GetColor() const { return m_crColor; }
  public: TextEditWindow* GetWindow() const { return m_pWindow; }

  // [I]
  public: bool IsStartActive() const { return m_fStartIsActive; }

  // [S]
  public: Color SetBackground(Color cr) { return m_crBackground = cr; }
  public: Color SetColor(Color cr) { return m_crColor = cr; }
  public: void SetStartIsActive(bool new_start_is_active);

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

#endif //!defined(INCLUDE_listener_winapp_visual_selection_h)
