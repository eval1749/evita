// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_SELECTOR_H_
#define EVITA_CSS_STYLE_SELECTOR_H_

#include "base/macros.h"
#include "common/strings/atomic_string.h"

namespace css {

class StyleSelector {
 public:
  static const common::AtomicString& active_selection();
  static const common::AtomicString& bad_grammar();
  static const common::AtomicString& comment();
  static const common::AtomicString& defaults();
  static const common::AtomicString& end_of_file_marker();
  static const common::AtomicString& end_of_line_marker();
  static const common::AtomicString& html_attribute_name();
  static const common::AtomicString& html_attribute_value();
  static const common::AtomicString& html_comment();
  static const common::AtomicString& html_element_name();
  static const common::AtomicString& html_entity();
  static const common::AtomicString& identifier();
  static const common::AtomicString& ime_inactive1();
  static const common::AtomicString& ime_inactive2();
  static const common::AtomicString& ime_input();
  static const common::AtomicString& ime_active1();
  static const common::AtomicString& ime_active2();
  static const common::AtomicString& inactive_selection();
  static const common::AtomicString& keyword();
  static const common::AtomicString& keyword2();
  static const common::AtomicString& label();
  static const common::AtomicString& line_wrap_marker();
  static const common::AtomicString& literal();
  static const common::AtomicString& misspelled();
  static const common::AtomicString& normal();
  static const common::AtomicString& operators();
  static const common::AtomicString& string_literal();
  static const common::AtomicString& tab_marker();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(StyleSelector);
};

}  // namespace css

#endif  // EVITA_CSS_STYLE_SELECTOR_H_
