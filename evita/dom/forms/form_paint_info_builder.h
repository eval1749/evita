// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_PAINT_INFO_BUILDER_H_
#define EVITA_DOM_FORMS_FORM_PAINT_INFO_BUILDER_H_

#include "base/macros.h"
#include "evita/dom/forms/form_paint_info.h"

namespace domapi {
enum class CaretShape;
}

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormPaintInfo::Builder
//
class FormPaintInfo::Builder final {
 public:
  Builder();
  ~Builder();

  void set_caret_shape(domapi::CaretShape caret_shape);
  void set_hovered_control(FormControl* control);
  void set_interactive(bool value);

  const FormPaintInfo& Build() const { return paint_info_; }

 private:
  FormPaintInfo paint_info_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_PAINT_INFO_BUILDER_H_
