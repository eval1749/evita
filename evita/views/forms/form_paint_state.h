// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_PAINT_STATE_H_
#define EVITA_VIEWS_FORMS_FORM_PAINT_STATE_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"

namespace domapi {
class Form;
class FormControl;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormPaintState
//
class FormPaintState final {
 public:
  FormPaintState();
  ~FormPaintState();

  bool IsChanged(const domapi::FormControl& control) const;
  void Update(std::unique_ptr<domapi::Form> form);

 private:
  std::unique_ptr<domapi::Form> form_;
  std::unordered_map<int, domapi::FormControl*> control_map_;

  DISALLOW_COPY_AND_ASSIGN(FormPaintState);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_PAINT_STATE_H_
