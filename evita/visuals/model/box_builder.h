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

//////////////////////////////////////////////////////////////////////
//
// BoxBuilder
//
class BoxBuilder final {
 public:
  explicit BoxBuilder(Box* box);
  ~BoxBuilder();

  std::unique_ptr<Box> Finish();

  BoxBuilder& Append(std::unique_ptr<Box> child);
  // TextBox
  BoxBuilder& SetBaseline(float baseline);
  BoxBuilder& SetColor(const FloatColor& color);

 private:
  std::unique_ptr<Box> box_;

  DISALLOW_COPY_AND_ASSIGN(BoxBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_BUILDER_H_
