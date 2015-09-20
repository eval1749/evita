// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_MUTATION_OBSERVER_CONTROLLER_H_
#define EVITA_DOM_TEXT_MUTATION_OBSERVER_CONTROLLER_H_

#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "common/memory/singleton.h"

namespace dom {

class Document;
class MutationObserver;
class MutationRecord;

class MutationObserverController final
    : public common::Singleton<MutationObserverController> {
  DECLARE_SINGLETON_CLASS(MutationObserverController);

 public:
  ~MutationObserverController() final;

  void Register(MutationObserver* observer, Document* document);
  std::vector<MutationRecord*> TakeRecords(MutationObserver* observer);
  void Unregister(MutationObserver* observer);

 private:
  class Tracker;

  MutationObserverController();

  std::unordered_map<Document*, Tracker*> map_;

  DISALLOW_COPY_AND_ASSIGN(MutationObserverController);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_MUTATION_OBSERVER_CONTROLLER_H_
