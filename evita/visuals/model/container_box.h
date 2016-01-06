// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
#define EVITA_VISUALS_MODEL_CONTAINER_BOX_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "evita/visuals/model/box.h"
#include "evita/visuals/model/children.h"

namespace visuals {

class BoxEditor;

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
class ContainerBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(ContainerBox, Box);

 public:
  ~ContainerBox() override;

  Children child_boxes() const { return Children(*this); }
  Box* first_child() const { return first_child_; }
  bool is_child_changed() const { return is_child_changed_; }
  Box* last_child() const { return last_child_; }

 protected:
  ContainerBox(RootBox* root_box, const Node* node);
  explicit ContainerBox(RootBox* root_box);

 private:
  friend class BoxEditor;

  // For ease of using list of child boxes, we don't use |std::unique_ptr<Box>|
  Box* first_child_ = nullptr;
  Box* last_child_ = nullptr;

  bool is_child_changed_ = false;

  DISALLOW_COPY_AND_ASSIGN(ContainerBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
