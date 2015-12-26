// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
#define EVITA_VISUALS_MODEL_CONTAINER_BOX_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "evita/visuals/model/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
class ContainerBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(ContainerBox, Box);

 public:
  ~ContainerBox() override;

  const std::vector<Box*> child_boxes() const { return child_boxes_; }

  template <typename T>
  T* AppendChild(std::unique_ptr<T>&& child) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    const auto& result = child.get();
    AppendChild(std::unique_ptr<Box>(child.release()));
    return result;
  }

  Box* AppendChild(std::unique_ptr<Box> child);

  template <typename T, typename... Args>
  T* AppendNew(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    const auto& child = new T(args...);
    AppendChild(std::unique_ptr<Box>(child));
    return child;
  }

  std::unique_ptr<Box> RemoveChild(Box* child);

 protected:
  ContainerBox();

 private:
  // For ease of using list of child boxes, we don't use |std::unique_ptr<Box>|
  // for elements of |std::vector<T>|.
  std::vector<Box*> child_boxes_;

  DISALLOW_COPY_AND_ASSIGN(ContainerBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CONTAINER_BOX_H_
