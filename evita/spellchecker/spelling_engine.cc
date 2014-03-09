// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/spellchecker/spelling_engine.h"

#include "evita/spellchecker/hunspell_engine.h"

namespace spellchecker {

SpellingEngine::SpellingEngine() {
}

SpellingEngine::~SpellingEngine() {
}

SpellingEngine* SpellingEngine::GetSpellingEngine() {
  return HunspellEngine::instance();
}

}  // namespace spellchecker
