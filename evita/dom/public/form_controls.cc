// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/dom/public/form_controls.h"

#include "evita/dom/public/caret_shape.h"
#include "evita/dom/public/form_control_visitor.h"

namespace domapi {

#define V(Name) \
  void Name::Accept(FormControlVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_DOMAPI_FORM_CONTROL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// FormControl::State
//
FormControl::State::State(const State& other) : values_(other.values_) {}

FormControl::State::State() {}
FormControl::State::~State() {}

bool FormControl::State::operator==(const State& other) const {
  return values_ == other.values_;
}

bool FormControl::State::operator!=(const State& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream,
                         const FormControl::State& state) {
  ostream << '{';
  auto delimiter = "";

#define V(Name, name)              \
  if (state.name()) {              \
    delimiter = ", ";              \
    ostream << delimiter << #name; \
  }
  FOR_EACH_DOMAPI_FORM_CONTROL_STATE(V)
#undef V

  return ostream << '}';
}

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
FormControl::FormControl(int id, const IntRect& bounds, const State& state)
    : bounds_(bounds), id_(id), state_(state) {}

FormControl::~FormControl() {}

std::ostream& operator<<(std::ostream& ostream, FormControl& control) {
  return ostream << control.class_name() << '(' << control.id() << ','
                 << control.state() << ',' << control.bounds() << ')';
}

bool FormControl::operator==(const FormControl& other) const {
  if (this == &other)
    return true;
  if (class_name() != other.class_name() || id_ != other.id_ ||
      bounds_ != other.bounds_ || state_ != other.state_) {
    return false;
  }
  return InternalEqualsTo(other);
}

bool FormControl::operator!=(const FormControl& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// Button
//
Button::Button(int id,
               const IntRect& bounds,
               const State& state,
               base::StringPiece16 label)
    : FormControl(id, bounds, state), label_(label.as_string()) {}

Button::~Button() {}

// FormControl
bool Button::InternalEqualsTo(const FormControl& other) const {
  return label_ == other.as<Button>()->label_;
}

//////////////////////////////////////////////////////////////////////
//
// Checkbox
//
Checkbox::Checkbox(int id, const IntRect& bounds, const State& state)
    : FormControl(id, bounds, state) {}

Checkbox::~Checkbox() {}

// FormControl
bool Checkbox::InternalEqualsTo(const FormControl& other) const {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// Label
//
Label::Label(int id,
             const IntRect& bounds,
             const State& state,
             base::StringPiece16 text)
    : FormControl(id, bounds, state), text_(text.as_string()) {}

Label::~Label() {}

// FormControl
bool Label::InternalEqualsTo(const FormControl& other) const {
  return text_ == other.as<Label>()->text_;
}

//////////////////////////////////////////////////////////////////////
//
// RadioButton
//
RadioButton::RadioButton(int id, const IntRect& bounds, const State& state)
    : FormControl(id, bounds, state) {}

RadioButton::~RadioButton() {}

// FormControl
bool RadioButton::InternalEqualsTo(const FormControl& other) const {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// TextField::Selection
//
TextField::Selection::Selection(const Selection& other)
    : anchor_offset_(other.anchor_offset_),
      focus_offset_(other.focus_offset_),
      caret_shape_(other.caret_shape_) {}

TextField::Selection::Selection() : caret_shape_(CaretShape::None) {}
TextField::Selection::~Selection() {}

TextField::Selection& TextField::Selection::operator=(const Selection& other) {
  anchor_offset_ = other.anchor_offset_;
  focus_offset_ = other.focus_offset_;
  return *this;
}

bool TextField::Selection::operator==(const Selection& other) const {
  return anchor_offset_ == other.anchor_offset_ &&
         focus_offset_ == other.focus_offset_;
}

bool TextField::Selection::operator!=(const Selection& other) const {
  return !operator==(other);
}

void TextField::Selection::Collapse(size_t offset) {
  anchor_offset_ = focus_offset_ = offset;
}

void TextField::Selection::Extend(size_t offset) {
  focus_offset_ = offset;
}

std::ostream& operator<<(std::ostream& ostream,
                         const TextField::Selection& selection) {
  return ostream << "Selection(" << selection.anchor_offset() << ','
                 << selection.focus_offset() << ')';
}

//////////////////////////////////////////////////////////////////////
//
// TextField
//
TextField::TextField(int id,
                     const IntRect& bounds,
                     const State& state,
                     const Selection& selection,
                     float scroll_left,
                     base::StringPiece16 text)
    : FormControl(id, bounds, state),
      scroll_left_(scroll_left),
      selection_(selection),
      text_(text.as_string()) {}

TextField::~TextField() {}

// FormControl
bool TextField::InternalEqualsTo(const FormControl& other) const {
  const auto other_text_field = other.as<TextField>();
  return scroll_left_ == other_text_field->scroll_left_ &&
         selection_ == other_text_field->selection_ &&
         text_ == other_text_field->text_;
}

}  // namespace domapi
