// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_spellchecker_hunspell_engine_h)
#define INCLUDE_evita_spellchecker_hunspell_engine_h

#include <memory>

#include "common/memory/singleton.h"
#include "evita/spellchecker/spelling_engine.h"

class Hunspell;

namespace base {
class Lock;
}

namespace spellchecker {

class HunspellEngine : public common::Singleton<HunspellEngine>,
                       public SpellingEngine {
  DECLARE_SINGLETON_CLASS(HunspellEngine);

  private: class Dictionary;

  private: std::unique_ptr<Dictionary> dictionary_;
  private: std::unique_ptr<Hunspell> hunspell_;
  private: std::unique_ptr<base::Lock> lock_;

  private: HunspellEngine();
  private: virtual ~HunspellEngine();

  // spellchecker::SpellingEngine
  private: virtual bool CheckSpelling(
      const base::string16& word_to_check) override;
  private: virtual bool EnsureInitialized() override;
  private: virtual std::vector<base::string16> GetSpellingSuggestions(
      const base::string16& wrong_word) override;

  DISALLOW_COPY_AND_ASSIGN(HunspellEngine);
};

}  // namespace spellchecker

#endif //!defined(INCLUDE_evita_spellchecker_hunspell_engine_h)
