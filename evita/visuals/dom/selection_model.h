// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SELECTION_MODEL_H_
#define EVITA_VISUALS_DOM_SELECTION_MODEL_H_

namespace visuals {

class ContainerNode;
class Node;

//////////////////////////////////////////////////////////////////////
//
// SelectionModel
//
class SelectionModel final {
 public:
  SelectionModel(const SelectionModel& other);
  SelectionModel();
  ~SelectionModel();

  bool operator==(const SelectionModel& other) const;
  bool operator!=(const SelectionModel& other) const;

  const Node& anchor_node() const;
  int anchor_offset() const;
  const Node& focus_node() const;
  int focus_offset() const;

  bool is_caret() const;
  bool is_none() const;
  bool is_range() const;

  void Clear();
  void Collapse(Node* node, int offset);
  void ExtendTo(Node* node, int offset);
  void WillRemoveChild(const ContainerNode& parent, const Node& child);

 private:
  Node* anchor_node_ = nullptr;
  int anchor_offset_ = 0;
  Node* focus_node_ = nullptr;
  int focus_offset_ = 0;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SELECTION_MODEL_H_
