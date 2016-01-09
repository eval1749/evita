// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_CONTROLLER_H_
#define EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_CONTROLLER_H_

#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "common/memory/singleton.h"

namespace dom {

class TextDocument;
class TextMutationObserver;
class TextMutationRecord;

class TextMutationObserverController final
    : public common::Singleton<TextMutationObserverController> {
  DECLARE_SINGLETON_CLASS(TextMutationObserverController);

 public:
  ~TextMutationObserverController() final;

  void Register(TextMutationObserver* observer, TextDocument* document);
  std::vector<TextMutationRecord*> TakeRecords(TextMutationObserver* observer);
  void Unregister(TextMutationObserver* observer);

 private:
  class Tracker;

  TextMutationObserverController();

  std::unordered_map<TextDocument*, Tracker*> map_;

  DISALLOW_COPY_AND_ASSIGN(TextMutationObserverController);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_CONTROLLER_H_
