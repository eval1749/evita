// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_INLINE_BOX_H_
#define EVITA_VISUALS_MODEL_INLINE_BOX_H_

#include <vector>

#include "evita/visuals/model/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
class InlineBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(InlineBox, Box);

 public:
  ~InlineBox() override;

 protected:
  explicit InlineBox(const FloatRect& bounds);

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_INLINE_BOX_H_
