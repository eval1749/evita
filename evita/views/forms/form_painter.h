// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_PAINTER_H_
#define EVITA_VIEWS_FORMS_FORM_PAINTER_H_

#include "base/macros.h"

namespace domapi {
class Form;
}

namespace views {

class FormPaintInfo;

//////////////////////////////////////////////////////////////////////
//
// FormPainter
//
class FormPainter final {
 public:
  FormPainter();
  ~FormPainter();

  void Paint(const FormPaintInfo& paint_info, const domapi::Form& form);

 private:
  DISALLOW_COPY_AND_ASSIGN(FormPainter);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_PAINTER_H_
