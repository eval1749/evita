// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/base/selection_model.h"

#include <algorithm>

namespace ui {

SelectionModel::SelectionModel(int size)
    : anchor_(-1), focus_(-1), size_(size) {
}

SelectionModel::~SelectionModel() {
}

void SelectionModel::Add(int index) {
  anchor_ = focus_ = index;
  set_.insert(index);
}

void SelectionModel::Clear() {
  anchor_ = focus_ = -1;
  set_.clear();
}

void SelectionModel::CollapseTo(int index) {
  anchor_ = focus_ = index;
  set_.clear();
  set_.insert(index);
}

void SelectionModel::DidAddItem(int index) {
  if (anchor_ >= index)
    ++anchor_;
  if (focus_ >= index)
    ++focus_;
  ++size_;
  std::unordered_set<int> old_set(set_);
  set_.clear();
  for (auto member : old_set) {
    set_.insert(member >= index ? member + 1 : member);
  }
}

void SelectionModel::DidRemoveItem(int index) {
  --size_;
  if (anchor_ >= index)
    anchor_ = std::max(anchor_ - 1, 0);
  if (focus_ >= index)
    focus_ = std::max(focus_ - 1, 0);
  set_.erase(index);
  if (anchor_ == -1)
    return;
  ExtendTo(focus_);
}

void SelectionModel::Extend(int direction) {
  if (anchor_ == -1)
    return;
  ExtendTo(std::max(std::min(focus_ + direction, size_ - 1), 0));
}

void SelectionModel::ExtendTo(int index) {
  if (anchor_ == -1) {
    CollapseTo(index);
    return;
  }
  focus_ = index;
  set_.clear();
  auto const start = std::min(anchor_, focus_);
  auto const end = std::max(anchor_, focus_);
  for (auto index = start; index <= end; ++index) {
    set_.insert(index);
  }
}

bool SelectionModel::IsSelected(int index) const {
  return set_.find(index) != set_.end();
}

void SelectionModel::Move(int direction) {
  if (focus_ == -1)
    return;
  CollapseTo(std::max(std::min(focus_ + direction, size_ - 1), 0));
}

}  // namespace ui
