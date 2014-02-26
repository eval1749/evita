// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style_selector.h"

namespace text {

#define DEFINE_STYLE_SELECTOR(name) \
  const base::string16& StyleSelector::name() { \
    DEFINE_STATIC_LOCAL(base::string16, name, (L ## #name)); \
    return name; \
  }

DEFINE_STYLE_SELECTOR(active_selection)
DEFINE_STYLE_SELECTOR(comment)
DEFINE_STYLE_SELECTOR(defaults)
DEFINE_STYLE_SELECTOR(end_of_file_marker)
DEFINE_STYLE_SELECTOR(end_of_line_marker)
DEFINE_STYLE_SELECTOR(inactive_selection)
DEFINE_STYLE_SELECTOR(line_wrap_marker)
DEFINE_STYLE_SELECTOR(keyword)
DEFINE_STYLE_SELECTOR(normal)
DEFINE_STYLE_SELECTOR(operators)
DEFINE_STYLE_SELECTOR(string_literal)
DEFINE_STYLE_SELECTOR(tab_marker)

}  // namespace text
