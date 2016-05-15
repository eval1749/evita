// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_MODELS_STYLE_SELECTOR_H_
#define EVITA_TEXT_STYLE_MODELS_STYLE_SELECTOR_H_

#include "base/macros.h"
#include "evita/base/strings/atomic_string.h"

namespace css {

class StyleSelector {
 public:
  static base::AtomicString active_selection();
  static base::AtomicString bad_grammar();
  static base::AtomicString comment();
  static base::AtomicString defaults();
  static base::AtomicString end_of_file_marker();
  static base::AtomicString end_of_line_marker();
  static base::AtomicString html_attribute_name();
  static base::AtomicString html_attribute_value();
  static base::AtomicString html_comment();
  static base::AtomicString html_element_name();
  static base::AtomicString html_entity();
  static base::AtomicString identifier();
  static base::AtomicString ime_inactive1();
  static base::AtomicString ime_inactive2();
  static base::AtomicString ime_input();
  static base::AtomicString ime_active1();
  static base::AtomicString ime_active2();
  static base::AtomicString inactive_selection();
  static base::AtomicString keyword();
  static base::AtomicString keyword2();
  static base::AtomicString label();
  static base::AtomicString line_wrap_marker();
  static base::AtomicString literal();
  static base::AtomicString misspelled();
  static base::AtomicString normal();
  static base::AtomicString operators();
  static base::AtomicString string_literal();
  static base::AtomicString tab_marker();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(StyleSelector);
};

}  // namespace css

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_SELECTOR_H_
