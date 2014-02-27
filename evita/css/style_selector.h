// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_selector_h)
#define INCLUDE_evita_css_style_selector_h

#include "base/strings/string16.h"

namespace text {

class StyleSelector {
  public: StyleSelector() = delete;
  public: ~StyleSelector() = delete;

  public: static const base::string16& active_selection();
  public: static const base::string16& comment();
  public: static const base::string16& defaults();
  public: static const base::string16& end_of_file_marker();
  public: static const base::string16& end_of_line_marker();
  public: static const base::string16& inactive_selection();
  public: static const base::string16& line_wrap_marker();
  public: static const base::string16& normal();
  public: static const base::string16& keyword();
  public: static const base::string16& operators();
  public: static const base::string16& string_literal();
  public: static const base::string16& tab_marker();
};

}  // namespace text

#endif //!defined(INCLUDE_evita_css_style_selector_h)
