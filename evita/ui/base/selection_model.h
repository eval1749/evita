// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_base_selection_model_h)
#define INCLUDE_evita_ui_base_selection_model_h

#include "base/strings/string16.h"

#include <unordered_set>

namespace ui {

class Selection {
  private: std::unordered_set<int> selected_set_;

  public: Selection();
  public: ~Selection();

  public: bool is_selected(int index) const;

  public: void Clear();
  public: void Extend(int direction);
  public: void ExtendTo(int index);
  public: void Select(int index);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_selection_model_h)
1