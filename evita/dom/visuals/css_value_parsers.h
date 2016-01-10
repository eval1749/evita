// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_VISUALS_CSS_VALUE_PARSERS_H_
#define EVITA_DOM_VISUALS_CSS_VALUE_PARSERS_H_

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "common/maybe.h"
#include "evita/visuals/css/values_forward.h"

namespace dom {

#define V(Name, name, name_string)               \
  common::Maybe<visuals::css::Name> Parse##Name( \
      const base::StringPiece16& text);          \
  base::string16 Unparse##Name(const visuals::css::Name& value);
FOR_EACH_VISUAL_CSS_VALUES(V)
#undef V

common::Maybe<visuals::css::Length> ParseLength(
    const base::StringPiece16& text);
common::Maybe<visuals::css::Percentage> ParsePercentage(
    const base::StringPiece16& text);

base::string16 UnparseLength(const visuals::css::Length& value);
base::string16 UnparsePercentage(const visuals::css::Percentage& value);

}  // namespace dom

#endif  // EVITA_DOM_VISUALS_CSS_VALUE_PARSERS_H_
