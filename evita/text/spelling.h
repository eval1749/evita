// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_spelling_h)
#define INCLUDE_evita_text_spelling_h

namespace text {

enum class Spelling {
  None,
  Corrected,
  Misspelled,
  BadGrammar,
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_spelling_h)
