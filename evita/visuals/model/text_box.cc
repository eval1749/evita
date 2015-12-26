// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
TextBox::TextBox(const base::string16 text) : text_(text) {}
TextBox::~TextBox() {}

FloatSize TextBox::ComputePreferredSize() const {
  // TODO(eval1749): Implement computing preferred size for |TextBox|.
  return FloatSize(text_.size() * 8.0f, 50) + border().size() +
         padding().size();
}

}  // namespace visuals
