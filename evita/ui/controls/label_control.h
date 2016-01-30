// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_LABEL_CONTROL_H_
#define EVITA_UI_CONTROLS_LABEL_CONTROL_H_

#include <memory>

#include "evita/ui/controls/control.h"

#include "base/strings/string16.h"
#include "evita/gfx/forward.h"

namespace ui {

class LabelControl final : public Control {
 public:
  LabelControl(ControlController* controller,
               const base::string16& text,
               const Style& style);
  ~LabelControl() final;

  bool focusable() const final;
  const Style& style() const { return style_; }
  void set_style(const Style& style);
  base::string16 text() const { return text_; }
  void set_text(const base::string16& text);

 private:
  // ui::Widget
  void OnDraw(gfx::Canvas* canvas) final;

  Style style_;
  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(LabelControl);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_LABEL_CONTROL_H_
