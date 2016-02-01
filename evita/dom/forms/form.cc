// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_observer.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_builder.h"
#include "evita/dom/public/form_controls.h"
#include "evita/ui/system_metrics.h"
#include "evita/visuals/fonts/font_description_builder.h"
#include "evita/visuals/fonts/text_format_factory.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Form
//
Form::Form() {
  ui::SystemMetrics::instance()->AddObserver(this);
}

Form::~Form() {
  ui::SystemMetrics::instance()->RemoveObserver(this);
}

std::vector<FormControl*> Form::controls() const {
  return controls_;
}

void Form::set_focus_control(v8_glue::Nullable<FormControl> new_focus_control) {
  if (focus_control_ == new_focus_control)
    return;
  focus_control_ = new_focus_control;
  NotifyChangeForm();
}

void Form::set_height(int new_height) {
  if (bounds_.height() == new_height)
    return;
  bounds_ =
      domapi::IntRect(bounds_.origin(), domapi::IntSize(width(), new_height));
  NotifyChangeForm();
}

void Form::set_title(const base::string16& new_title) {
  if (title_ == new_title)
    return;
  title_ = new_title;
  NotifyChangeForm();
}

void Form::set_width(int new_width) {
  if (bounds_.width() == new_width)
    return;
  bounds_ =
      domapi::IntRect(bounds_.origin(), domapi::IntSize(new_width, height()));
  NotifyChangeForm();
}

void Form::AddFormControl(FormControl* control,
                          ExceptionState* exception_state) {
  if (control->form_) {
    exception_state->ThrowError("Specified control is already added to form.");
    return;
  }
  controls_.push_back(control);
  control->form_ = this;
  NotifyChangeForm();
}

void Form::AddObserver(FormObserver* observer) const {
  observers_.AddObserver(observer);
}

void Form::DidChangeFormControl(FormControl* control) {
  DCHECK(control);
  NotifyChangeForm();
}

const visuals::TextFormat& Form::GetTextFormat() const {
  const auto& metrics = *ui::SystemMetrics::instance();
  visuals::FontDescription::Builder font_desc;
  font_desc.SetFamily(metrics.font_family());
  font_desc.SetSize(metrics.font_size());
  return visuals::TextFormatFactory::GetInstance()->Get(font_desc.Build());
}

void Form::NotifyChangeForm() {
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

std::unique_ptr<domapi::Form> Form::Paint(
    const FormPaintInfo& paint_info) const {
  domapi::Form::Builder builder(event_target_id());
  builder.SetBounds(bounds_).SetTitle(title_);
  for (const auto& control : controls_)
    builder.AddControl(std::move(control->Paint(paint_info)));
  return builder.Build();
}

void Form::RemoveObserver(FormObserver* observer) const {
  observers_.RemoveObserver(observer);
}

// ui::SystemMetricsObserver
void Form::DidChangeIconFont() {
  NotifyChangeForm();
}

void Form::DidChangeSystemColor() {
  NotifyChangeForm();
}

void Form::DidChangeSystemMetrics() {
  NotifyChangeForm();
}

}  // namespace dom
