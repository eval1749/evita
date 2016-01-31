// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FORM_H_
#define EVITA_DOM_PUBLIC_FORM_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "common/castable.h"
#include "evita/dom/public/geometry.h"

namespace domapi {

class FormControl;

//////////////////////////////////////////////////////////////////////
//
// Form
//
// |Form| manages life time of |FormControl|.
//
class Form final {
 public:
  class Builder;

  Form();
  ~Form();

  bool operator==(const Form& other) const;
  bool operator!=(const Form& other) const;

  const IntRect& bounds() const { return bounds_; }
  const std::vector<FormControl*>& controls() const { return controls_; }
  const base::string16& title() const { return title_; }

 private:
  IntRect bounds_;
  std::vector<FormControl*> controls_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FORM_H_
