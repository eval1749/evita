// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_text_field_control_h)
#define INCLUDE_evita_ui_controls_text_field_control_h

#include "evita/ui/controls/control.h"

#include <memory>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

namespace ui {

class TextFieldControl final : public Control {
  public: struct Selection {
    int end;
    int start;
    bool start_is_active;

    bool operator==(const Selection& other) const;
    bool operator!=(const Selection& other) const;
  };
  private: class Renderer;

  private: std::unique_ptr<Renderer> renderer_;
  private: Selection selection_;
  private: Style style_;
  private: base::string16 text_;

  public: TextFieldControl(ControlController* controller,
                           const Selection& selection,
                           const base::string16& text, const Style& style);
  public: virtual ~TextFieldControl();

  public: const Style& style() const { return style_; }
  public: void set_style(const Style& style);
  public: base::string16 text() const { return text_; }
  public: void set_selection(const Selection& new_selection);
  public: void set_text(const base::string16& text);

  // ui::Widget
  private: virtual void DidResize() override;
  private: virtual void OnDraw(gfx::Graphics* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_text_field_control_h)
