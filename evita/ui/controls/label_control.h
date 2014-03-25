// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_label_control_h)
#define INCLUDE_evita_ui_controls_label_control_h

#include "evita/ui/controls/control.h"

#include <memory>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

namespace ui {

class LabelControl : public Control {
  public: struct LabelStyle {
    gfx::ColorF bgcolor;
    gfx::ColorF color;
    base::string16 font_family;
    float font_size;

    bool operator==(const LabelStyle& other) const;
    bool operator!=(const LabelStyle& other) const;
  };
  private: class Renderer;

  private: std::unique_ptr<Renderer> renderer_;
  private: LabelStyle style_;
  private: base::string16 text_;

  public: LabelControl(ControlController* controller,
                       const base::string16& text, const LabelStyle& style);
  public: virtual ~LabelControl();

  public: const LabelStyle& style() const { return style_; }
  public: void set_style(const LabelStyle& style);
  public: base::string16 text() const { return text_; }
  public: void set_text(const base::string16& text);

  // ui::Widget
  private: virtual void DidResize() override;
  private: virtual void OnDraw(gfx::Graphics* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(LabelControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_label_control_h)
