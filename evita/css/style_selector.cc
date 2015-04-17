// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_selector.h"

namespace css {

#define DEFINE_STYLE_SELECTOR(name) \
  const common::AtomicString& StyleSelector::name() { \
    CR_DEFINE_STATIC_LOCAL(common::AtomicString, name, (L ## #name)); \
    return name; \
  }

DEFINE_STYLE_SELECTOR(active_selection)
DEFINE_STYLE_SELECTOR(bad_grammar)
DEFINE_STYLE_SELECTOR(comment)
DEFINE_STYLE_SELECTOR(defaults)
DEFINE_STYLE_SELECTOR(end_of_file_marker)
DEFINE_STYLE_SELECTOR(end_of_line_marker)
DEFINE_STYLE_SELECTOR(html_attribute_name)
DEFINE_STYLE_SELECTOR(html_attribute_value)
DEFINE_STYLE_SELECTOR(html_comment)
DEFINE_STYLE_SELECTOR(html_element_name)
DEFINE_STYLE_SELECTOR(html_entity)
DEFINE_STYLE_SELECTOR(identifier)
DEFINE_STYLE_SELECTOR(ime_inactive1);
DEFINE_STYLE_SELECTOR(ime_inactive2);
DEFINE_STYLE_SELECTOR(ime_input);
DEFINE_STYLE_SELECTOR(ime_active1);
DEFINE_STYLE_SELECTOR(ime_active2);
DEFINE_STYLE_SELECTOR(inactive_selection)
DEFINE_STYLE_SELECTOR(label)
DEFINE_STYLE_SELECTOR(line_wrap_marker)
DEFINE_STYLE_SELECTOR(literal)
DEFINE_STYLE_SELECTOR(keyword)
DEFINE_STYLE_SELECTOR(keyword2)
DEFINE_STYLE_SELECTOR(misspelled)
DEFINE_STYLE_SELECTOR(normal)
DEFINE_STYLE_SELECTOR(operators)
DEFINE_STYLE_SELECTOR(string_literal)
DEFINE_STYLE_SELECTOR(tab_marker)

}  // namespace css
