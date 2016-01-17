// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SELECTION_H_
#define EVITA_VISUALS_DOM_SELECTION_H_

namespace visuals {

class Node;
class SelectionEdtior;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection final {
 public:
  Selection(const Selection& other);
  Selection();
  ~Selection();

  bool operator==(const Selection& other) const;
  bool operator!=(const Selection& other) const;

  Node* anchor_node() const { return anchor_node_; }
  int anchor_offset() const { return anchor_offset_; }
  Node* focus_node() const { return focus_node_; }
  int focus_offset() const { return focus_offset_; }
  bool is_caret() const;
  bool is_none() const;
  bool is_range() const;

 private:
  friend class SelectionEditor;

  Node* anchor_node_ = nullptr;
  int anchor_offset_ = 0;
  Node* focus_node_ = nullptr;
  int focus_offset_ = 0;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SELECTION_H_
