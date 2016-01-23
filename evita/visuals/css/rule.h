// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_RULE_H_
#define EVITA_VISUALS_CSS_RULE_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace visuals {
namespace css {

class Style;

//////////////////////////////////////////////////////////////////////
//
// Rule
//
class Rule final {
 public:
  Rule(base::StringPiece16 selector, std::unique_ptr<Style> style);
  ~Rule();

  const base::string16& selector() const { return selector_; }
  const Style& style() const { return *style_; }

 private:
  const base::string16 selector_;
  std::unique_ptr<Style> style_;

  DISALLOW_COPY_AND_ASSIGN(Rule);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_RULE_H_
