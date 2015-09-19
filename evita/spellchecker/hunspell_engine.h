// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_SPELLCHECKER_HUNSPELL_ENGINE_H_
#define EVITA_SPELLCHECKER_HUNSPELL_ENGINE_H_

#include <memory>
#include <vector>

#include "common/memory/singleton.h"
#include "evita/spellchecker/spelling_engine.h"

class Hunspell;

namespace base {
class Lock;
}

namespace spellchecker {

class HunspellEngine final : public common::Singleton<HunspellEngine>,
                             public SpellingEngine {
  DECLARE_SINGLETON_CLASS(HunspellEngine);

 private:
  class Dictionary;

  HunspellEngine();
  ~HunspellEngine() final;

  // spellchecker::SpellingEngine
  bool CheckSpelling(const base::string16& word_to_check) final;
  bool EnsureInitialized() final;
  std::vector<base::string16> GetSpellingSuggestions(
      const base::string16& wrong_word) final;

  std::unique_ptr<Dictionary> dictionary_;
  std::unique_ptr<Hunspell> hunspell_;
  std::unique_ptr<base::Lock> lock_;

  DISALLOW_COPY_AND_ASSIGN(HunspellEngine);
};

}  // namespace spellchecker

#endif  // EVITA_SPELLCHECKER_HUNSPELL_ENGINE_H_
