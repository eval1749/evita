// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_selector_h)
#define INCLUDE_evita_css_style_selector_h

#include "common/strings/atomic_string.h"

namespace css {

class StyleSelector {
  public: StyleSelector() = delete;
  public: ~StyleSelector() = delete;

  public: static const common::AtomicString& active_selection();
  public: static const common::AtomicString& bad_grammar();
  public: static const common::AtomicString& comment();
  public: static const common::AtomicString& defaults();
  public: static const common::AtomicString& end_of_file_marker();
  public: static const common::AtomicString& end_of_line_marker();
  public: static const common::AtomicString& html_attribute_name();
  public: static const common::AtomicString& html_attribute_value();
  public: static const common::AtomicString& html_comment();
  public: static const common::AtomicString& html_element_name();
  public: static const common::AtomicString& html_entity();
  public: static const common::AtomicString& identifier();
  public: static const common::AtomicString& ime_inactive1();
  public: static const common::AtomicString& ime_inactive2();
  public: static const common::AtomicString& ime_input();
  public: static const common::AtomicString& ime_active1();
  public: static const common::AtomicString& ime_active2();
  public: static const common::AtomicString& inactive_selection();
  public: static const common::AtomicString& keyword();
  public: static const common::AtomicString& keyword2();
  public: static const common::AtomicString& label();
  public: static const common::AtomicString& line_wrap_marker();
  public: static const common::AtomicString& literal();
  public: static const common::AtomicString& misspelled();
  public: static const common::AtomicString& normal();
  public: static const common::AtomicString& operators();
  public: static const common::AtomicString& string_literal();
  public: static const common::AtomicString& tab_marker();
};

}  // namespace css

#endif //!defined(INCLUDE_evita_css_style_selector_h)
