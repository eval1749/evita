// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_CONTENT_BOX_H_
#define EVITA_VISUALS_LAYOUT_CONTENT_BOX_H_

#include <vector>

#include "evita/visuals/layout/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContentBox
//
class ContentBox : public Box {
  DECLARE_VISUAL_BOX_ABSTRACT_CLASS(ContentBox, Box);

 public:
  ~ContentBox() override;

 protected:
  ContentBox(RootBox* root_box, const Node* node);
  explicit ContentBox(RootBox* root_box);

 private:
  DISALLOW_COPY_AND_ASSIGN(ContentBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_CONTENT_BOX_H_
