// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/base/ime/text_composition.h"

#include "base/logging.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// TextComposition::Span
//
TextComposition::Span::Span() : end(0), start(0), type(Type::Input) {}

//////////////////////////////////////////////////////////////////////
//
// TextComposition
//
TextComposition::TextComposition(const base::string16& text,
                                 const std::vector<Span> spans)
    : caret_(0), spans_(spans), text_(text) {}

TextComposition::TextComposition() {}

TextComposition::~TextComposition() {}

}  // namespace ui
