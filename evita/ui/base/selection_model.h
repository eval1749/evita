// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_UI_BASE_SELECTION_MODEL_H_
#define EVITA_UI_BASE_SELECTION_MODEL_H_

#include <unordered_set>

#include "base/strings/string16.h"

namespace ui {

class SelectionModel final {
 public:
  explicit SelectionModel(int size);
  ~SelectionModel();

  bool empty() const { return !set_.size(); }

  void Add(int index);
  void Clear();
  void CollapseTo(int index);
  void DidAddItem(int index);
  void DidRemoveItem(int index);
  void Extend(int direction);
  void ExtendTo(int index);
  bool IsSelected(int index) const;
  void Move(int direction);

 private:
  int anchor_;
  int focus_;
  int size_;
  std::unordered_set<int> set_;

  DISALLOW_COPY_AND_ASSIGN(SelectionModel);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_SELECTION_MODEL_H_
