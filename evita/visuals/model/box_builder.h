// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_BUILDER_H_
#define EVITA_VISUALS_MODEL_BOX_BUILDER_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class Box;
class FloatColor;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// BoxBuilder
//
class BoxBuilder final {
 public:
  BoxBuilder(BoxBuilder&& other) : box_(std::move(other.box_)) {}
  ~BoxBuilder();

  std::unique_ptr<Box> Finish();

  BoxBuilder& Append(std::unique_ptr<Box> child);
  BoxBuilder& SetStyle(const css::Style& style);

  // TextBox
  BoxBuilder& SetBaseline(float baseline);

  template <typename T, typename... Args>
  static BoxBuilder New(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return std::move(BoxBuilder(std::unique_ptr<Box>(new T(args...))));
  }

 private:
  explicit BoxBuilder(std::unique_ptr<Box> box);

  std::unique_ptr<Box> box_;

  DISALLOW_COPY_AND_ASSIGN(BoxBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_BUILDER_H_
