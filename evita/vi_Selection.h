// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_listener_winapp_visual_selection_h)
#define INCLUDE_listener_winapp_visual_selection_h

#include "evita/css/style.h"

class TextEditWindow;

namespace text {
class Range;
class Buffer;
}

class Selection {
  private: bool m_fStartIsActive;
  private: text::Range* m_pRange;

  public: Selection(text::Range* range);
  public: ~Selection();

  public: text::Range* range() const { return m_pRange; }

  // [G]
  public: Posn GetEnd() const;
  public: Posn GetStart() const;

  // [I]
  public: bool IsStartActive() const { return m_fStartIsActive; }

  // [S]
  public: void SetStartIsActive(bool new_start_is_active);

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

#endif //!defined(INCLUDE_listener_winapp_visual_selection_h)
