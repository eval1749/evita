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
  static const base::AtomicString& active_selection();
  static const base::AtomicString& bad_grammar();
  static const base::AtomicString& comment();
  static const base::AtomicString& defaults();
  static const base::AtomicString& end_of_file_marker();
  static const base::AtomicString& end_of_line_marker();
  static const base::AtomicString& html_attribute_name();
  static const base::AtomicString& html_attribute_value();
  static const base::AtomicString& html_comment();
  static const base::AtomicString& html_element_name();
  static const base::AtomicString& html_entity();
  static const base::AtomicString& identifier();
  static const base::AtomicString& ime_inactive1();
  static const base::AtomicString& ime_inactive2();
  static const base::AtomicString& ime_input();
  static const base::AtomicString& ime_active1();
  static const base::AtomicString& ime_active2();
  static const base::AtomicString& inactive_selection();
  static const base::AtomicString& keyword();
  static const base::AtomicString& keyword2();
  static const base::AtomicString& label();
  static const base::AtomicString& line_wrap_marker();
  static const base::AtomicString& literal();
  static const base::AtomicString& misspelled();
  static const base::AtomicString& normal();
  static const base::AtomicString& operators();
  static const base::AtomicString& string_literal();
  static const base::AtomicString& tab_marker();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(StyleSelector);
};

}  // namespace css

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_SELECTOR_H_
