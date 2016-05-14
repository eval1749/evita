// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_SELECTOR_PARSER_H_
#define EVITA_VISUALS_CSS_SELECTOR_PARSER_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/css/selector.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Selector::Parser
//
class Selector::Parser {
 public:
  Parser();
  ~Parser();

  base::StringPiece16 error() const { return error_; }
  size_t position() const { return position_; }

  Selector Parse(base::StringPiece16 text);

 private:
  Selector Error(base::StringPiece16 error, size_t index);

  base::StringPiece16 error_;
  size_t position_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_SELECTOR_PARSER_H_
