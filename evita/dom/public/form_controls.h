// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FORM_CONTROLS_H_
#define EVITA_DOM_PUBLIC_FORM_CONTROLS_H_

#include <bitset>
#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "common/castable.h"
#include "evita/dom/public/form_controls_forward.h"
#include "evita/dom/public/geometry.h"

namespace domapi {

enum class CaretShape;

#define DECLARE_DOMAPI_FORM_MODEL_CLASS(name, base_name) \
  DECLARE_CASTABLE_CLASS(name, base_name)

#define DECLARE_ABSTRACT_DOM_FORM_MODEL_CLASS(name, base_name) \
  DECLARE_DOMAPI_FORM_MODEL_CLASS(name, base_name)

#define DECLARE_FINAL_DOM_FORM_MODEL_CLASS(name, base_name) \
  DECLARE_DOMAPI_FORM_MODEL_CLASS(name, base_name)          \
  void Accept(FormControlVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
class FormControl : public common::Castable<FormControl> {
  DECLARE_ABSTRACT_DOM_FORM_MODEL_CLASS(FormControl, Castable);

 public:
  class State final {
   public:
    State(const State& other);
    State();
    ~State();

    bool operator==(const State& other) const;
    bool operator!=(const State& other) const;

#define V(Name, name)                                     \
  bool name() const { return values_.test(Index::Name); } \
  void reset_##name() { values_.reset(Index::Name); }     \
  void set_##name() { values_.set(Index::Name); }

    FOR_EACH_DOMAPI_FORM_CONTROL_STATE(V)
#undef V

   private:
    enum Index {
#define V(Name, name) Name,
      FOR_EACH_DOMAPI_FORM_CONTROL_STATE(V)
#undef V
          Size,
    };

    std::bitset<static_cast<size_t>(Index::Size)> values_;
  };

  virtual ~FormControl();

  bool operator==(const FormControl& other) const;
  bool operator!=(const FormControl& other) const;

  const IntRect& bounds() const { return bounds_; }
  int id() const { return id_; }
  const State& state() const { return state_; }

  virtual void Accept(FormControlVisitor* visitor) = 0;

 protected:
  FormControl(int id, const IntRect& bounds, const State& state);

  virtual bool InternalEqualsTo(const FormControl& other) const = 0;

 private:
  const IntRect bounds_;
  const int id_;
  const State state_;

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

std::ostream& operator<<(std::ostream& ostream, const FormControl& control);
std::ostream& operator<<(std::ostream& ostream, const FormControl* control);
std::ostream& operator<<(std::ostream& ostream,
                         const FormControl::State& state);

//////////////////////////////////////////////////////////////////////
//
// Button
//
class Button final : public FormControl {
  DECLARE_FINAL_DOM_FORM_MODEL_CLASS(Button, FormControl);

 public:
  Button(int id,
         const IntRect& bounds,
         const State& state,
         base::StringPiece16 label);
  ~Button() final;

  const base::string16& label() const { return label_; }

 private:
  // FormControl
  bool InternalEqualsTo(const FormControl& other) const final;

  const base::string16 label_;

  DISALLOW_COPY_AND_ASSIGN(Button);
};

//////////////////////////////////////////////////////////////////////
//
// Checkbox
//
class Checkbox final : public FormControl {
  DECLARE_FINAL_DOM_FORM_MODEL_CLASS(Checkbox, FormControl);

 public:
  Checkbox(int id, const IntRect& bounds, const State& state);
  ~Checkbox() final;

  bool checked() const { return state().checked(); }

 private:
  // FormControl
  bool InternalEqualsTo(const FormControl& other) const final;

  DISALLOW_COPY_AND_ASSIGN(Checkbox);
};

//////////////////////////////////////////////////////////////////////
//
// Label
//
class Label final : public FormControl {
  DECLARE_FINAL_DOM_FORM_MODEL_CLASS(Label, FormControl);

 public:
  Label(int id,
        const IntRect& bounds,
        const State& state,
        base::StringPiece16 text);
  ~Label() final;

  const base::string16& text() const { return text_; }

 private:
  // FormControl
  bool InternalEqualsTo(const FormControl& other) const final;

  const base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(Label);
};

//////////////////////////////////////////////////////////////////////
//
// RadioButton
//
class RadioButton final : public FormControl {
  DECLARE_FINAL_DOM_FORM_MODEL_CLASS(RadioButton, FormControl);

 public:
  RadioButton(int id, const IntRect& bounds, const State& state);
  ~RadioButton() final;

  bool checked() const { return state().checked(); }

 private:
  // FormControl
  bool InternalEqualsTo(const FormControl& other) const final;

  DISALLOW_COPY_AND_ASSIGN(RadioButton);
};

//////////////////////////////////////////////////////////////////////
//
// TextField
//
class TextField final : public FormControl {
  DECLARE_FINAL_DOM_FORM_MODEL_CLASS(TextField, FormControl);

 public:
  class Selection final {
   public:
    Selection(const Selection& other);
    Selection();
    ~Selection();

    Selection& operator=(const Selection& other);

    bool operator==(const Selection& other) const;
    bool operator!=(const Selection& other) const;

    size_t anchor_offset() const { return anchor_offset_; }
    CaretShape caret_shape() const { return caret_shape_; }
    void set_caret_shape(CaretShape new_shape) { caret_shape_ = new_shape; }
    bool collapsed() const { return anchor_offset_ == focus_offset_; }
    size_t focus_offset() const { return focus_offset_; }

    void Collapse(size_t offset);
    void Extend(size_t offset);

   private:
    size_t anchor_offset_ = 0;
    size_t focus_offset_ = 0;
    CaretShape caret_shape_;
  };

  TextField(int id,
            const IntRect& bounds,
            const State& state,
            const Selection& selection,
            float scroll_left,
            base::StringPiece16 text);
  ~TextField() final;

  float scroll_left() const { return scroll_left_; }
  const Selection& selection() const { return selection_; }
  const base::string16& text() const { return text_; }

 private:
  // FormControl
  bool InternalEqualsTo(const FormControl& other) const final;

  const float scroll_left_;
  const Selection selection_;
  const base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(TextField);
};

std::ostream& operator<<(std::ostream& ostream,
                         const TextField::Selection& selection);

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FORM_CONTROLS_H_
