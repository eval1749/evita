// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/form_control_visitor.h"

#include "evita/dom/public/form_controls.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// FormControlVisitor
//
FormControlVisitor::FormControlVisitor() {}
FormControlVisitor::~FormControlVisitor() {}

void FormControlVisitor::Visit(const FormControl& control) {
  const_cast<FormControl&>(control).Accept(this);
}

}  // namespace domapi
