// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_spellchecker_spelling_engine_h)
#define INCLUDE_evita_spellchecker_spelling_engine_h

#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace spellchecker {

class SpellingEngine {
  protected: SpellingEngine();
  public: virtual ~SpellingEngine();

  public: static SpellingEngine* GetSpellingEngine();

  public: virtual bool CheckSpelling(const base::string16& word_to_check) = 0;
  public: virtual bool EnsureInitialized() = 0;
  public: virtual std::vector<base::string16> GetSpellingSuggestions(
      const base::string16& wrong_word) = 0;

  DISALLOW_COPY_AND_ASSIGN(SpellingEngine);
};

}  // namespace spellchecker

#endif //!defined(INCLUDE_evita_spellchecker_spelling_engine_h)
