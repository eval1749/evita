// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/form_builder.h"

#include "base/logging.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_controls.h"

namespace domapi {

using Builder = Form::Builder;

//////////////////////////////////////////////////////////////////////
//
// FormBuilder
//
Builder::Builder() : form_(new Form()) {}

Builder::~Builder() {
  DCHECK(!form_);
}

Builder& Builder::AddControl(std::unique_ptr<FormControl> control) {
  form_->controls_.emplace_back(control.release());
  return *this;
}

std::unique_ptr<Form> Builder::Build() {
  return std::move(form_);
}

Builder& Builder::SetBounds(const IntRect& bounds) {
  DCHECK(form_->bounds_.IsEmpty());
  form_->bounds_ = bounds;
  return *this;
}

Builder& Builder::SetTitle(base::StringPiece16 title) {
  DCHECK(form_->title_.empty());
  form_->title_ = title.as_string();
  return *this;
}

}  // namespace domapi
