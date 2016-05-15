// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_SELECTOR_BUILDER_H_
#define EVITA_VISUALS_CSS_SELECTOR_BUILDER_H_

#include <set>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/css/selector.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Selector::Builder
//
class Selector::Builder {
 public:
  Builder();
  ~Builder();

  Builder& AddClass(base::AtomicString class_name);
  Builder& AddClass(base::StringPiece16 class_name);
  Selector Build();
  Builder& SetId(base::AtomicString id);
  Builder& SetId(base::StringPiece16 id);
  Builder& SetTagName(base::AtomicString tag_name);
  Builder& SetTagName(base::StringPiece16 tag_name);

  // Return universal selector from typed selector |selector|.
  static Selector CopyWithoutTagName(const Selector& selector);

 private:
  std::set<base::AtomicString> classes_;
  base::AtomicString id_;
  base::AtomicString tag_name_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_SELECTOR_BUILDER_H_
