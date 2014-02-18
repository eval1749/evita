// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_listener_winapp_visual_selection_h)
#define INCLUDE_listener_winapp_visual_selection_h

#include "evita/ed_Style.h"

class TextEditWindow;

namespace dom {
class Buffer;
}

namespace text {
class Range;
}

//////////////////////////////////////////////////////////////////////
//
// Selection
//
// m_lRestoreLineNum
// A line number which selection belongs before reloading. We try to
// move selection to this line after reloading.
//
class Selection {
  private: Color m_crColor;
  private: Color m_crBackground;
  private: bool m_fStartIsActive;
  private: dom::Buffer* m_pBuffer;
  private: text::Range* m_pRange;

  public: Selection(dom::Buffer* buffer, text::Range* range);
  public: ~Selection();

  public: text::Range* range() const { return m_pRange; }

  // [G]
  public: Color GetBackground() const { return m_crBackground; }
  public: dom::Buffer* GetBuffer() const { return m_pBuffer; }
  public: Posn GetEnd() const;
  public: Color GetColor() const { return m_crColor; }
  public: Posn GetStart() const;

  // [I]
  public: bool IsStartActive() const { return m_fStartIsActive; }

  // [S]
  public: Color SetBackground(Color cr) { return m_crBackground = cr; }
  public: Color SetColor(Color cr) { return m_crColor = cr; }
  public: void SetStartIsActive(bool new_start_is_active);

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

#endif //!defined(INCLUDE_listener_winapp_visual_selection_h)
