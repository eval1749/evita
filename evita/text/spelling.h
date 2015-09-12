// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_SPELLING_H_
#define EVITA_TEXT_SPELLING_H_

namespace text {

enum class Spelling {
  None,
  Corrected,
  Misspelled,
  BadGrammar,
};

}  // namespace text

#endif  // EVITA_TEXT_SPELLING_H_
