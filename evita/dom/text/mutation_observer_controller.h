// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mutation_observer_controller_h)
#define INCLUDE_evita_dom_mutation_observer_controller_h

#include <unordered_map>

#include "base/macros.h"
#include "common/memory/singleton.h"

namespace dom {

class Document;
class MutationObserver;
class MutationRecord;

class MutationObserverController final
    : public common::Singleton<MutationObserverController> {
  DECLARE_SINGLETON_CLASS(MutationObserverController);

  private: class Tracker;

  private: std::unordered_map<Document*, Tracker*> map_;

  private: MutationObserverController();
  public: virtual ~MutationObserverController();

  public: void Register(MutationObserver* observer, Document* document);
  public: std::vector<MutationRecord*> TakeRecords(MutationObserver* observer);
  public: void Unregister(MutationObserver* observer);

  DISALLOW_COPY_AND_ASSIGN(MutationObserverController);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_mutation_observer_controller_h)
