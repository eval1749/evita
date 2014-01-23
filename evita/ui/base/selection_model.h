// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_base_selection_model_h)
#define INCLUDE_evita_ui_base_selection_model_h

#include "base/strings/string16.h"

#include <unordered_set>

namespace ui {

class SelectionModel {
  private: int anchor_;
  private: int focus_;
  private: int size_;
  private: std::unordered_set<int> set_;

  public: SelectionModel(int size);
  public: ~SelectionModel();

  public: bool empty() const { return !set_.size(); }

  public: void Add(int index);
  public: void Clear();
  public: void CollapseTo(int index);
  public: void DidAddItem();
  public: void DidRemoveItem(int index);
  public: void Extend(int direction);
  public: void ExtendTo(int index);
  public: bool IsSelected(int index) const;
  public: void Move(int direction);

  DISALLOW_COPY_AND_ASSIGN(SelectionModel);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_selection_model_h)
