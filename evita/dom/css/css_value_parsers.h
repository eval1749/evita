// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CSS_CSS_VALUE_PARSERS_H_
#define EVITA_DOM_CSS_CSS_VALUE_PARSERS_H_

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/base/maybe.h"
#include "evita/css/values_forward.h"

namespace dom {

#define V(Name, name, name_string)                              \
  base::Maybe<css::Name> Parse##Name(base::StringPiece16 text); \
  base::string16 Unparse##Name(const css::Name& value);
FOR_EACH_VISUAL_CSS_VALUE(V)
#undef V

base::Maybe<css::ColorValue> ParseColorValue(base::StringPiece16 text);
base::Maybe<css::Length> ParseLength(base::StringPiece16 text);
base::Maybe<css::Percentage> ParsePercentage(base::StringPiece16 text);
base::Maybe<css::String> ParseString(base::StringPiece16 text);

base::string16 UnparseLength(const css::Length& value);
base::string16 UnparsePercentage(const css::Percentage& value);
base::string16 UnparseString(const css::String& value);

}  // namespace dom

#endif  // EVITA_DOM_CSS_CSS_VALUE_PARSERS_H_
