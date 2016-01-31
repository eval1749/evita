// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_PAINT_INFO_H_
#define EVITA_DOM_FORMS_FORM_PAINT_INFO_H_

#include "base/macros.h"

namespace domapi {
enum class CaretShape;
}

namespace dom {

class FormControl;

//////////////////////////////////////////////////////////////////////
//
// FormPaintInfo
//
class FormPaintInfo final {
 public:
  class Builder;

  FormPaintInfo();
  ~FormPaintInfo();

  domapi::CaretShape caret_shape() const { return caret_shape_; }
  FormControl* hovered_control() const { return hovered_control_; }
  bool is_interactive() const { return is_interactive_; }

 private:
  domapi::CaretShape caret_shape_;
  FormControl* hovered_control_ = nullptr;
  bool is_interactive_ = false;

  DISALLOW_COPY_AND_ASSIGN(FormPaintInfo);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_PAINT_INFO_H_
