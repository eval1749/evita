// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FORM_BUILDER_H_
#define EVITA_DOM_PUBLIC_FORM_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/dom/public/geometry.h"
#include "evita/dom/public/form.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// Form::Builder
//
class Form::Builder final {
 public:
  Builder();
  ~Builder();

  Builder& AddControl(std::unique_ptr<FormControl> control);
  std::unique_ptr<Form> Build();
  Builder& SetBounds(const IntRect& bounds);
  Builder& SetTitle(base::StringPiece16 title);

 private:
  std::unique_ptr<Form> form_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FORM_BUILDER_H_
