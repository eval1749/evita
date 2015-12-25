// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_SPELLCHECKER_SPELLING_ENGINE_H_
#define EVITA_SPELLCHECKER_SPELLING_ENGINE_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace spellchecker {

class SpellingEngine {
 public:
  virtual ~SpellingEngine();

  static SpellingEngine* GetSpellingEngine();

  virtual bool CheckSpelling(const base::string16& word_to_check) = 0;
  virtual bool EnsureInitialized() = 0;
  virtual std::vector<base::string16> GetSpellingSuggestions(
      const base::string16& wrong_word) = 0;

 protected:
  SpellingEngine();

 private:
  DISALLOW_COPY_AND_ASSIGN(SpellingEngine);
};

}  // namespace spellchecker

#endif  // EVITA_SPELLCHECKER_SPELLING_ENGINE_H_
