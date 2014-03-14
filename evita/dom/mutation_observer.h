// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mutation_observer_h)
#define INCLUDE_evita_dom_mutation_observer_h

#include <unordered_map>
#include <vector>

#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class Document;
class MutationObserverInit;
class MutationRecord;

class MutationObserver : public v8_glue::Scriptable<MutationObserver> {
  DECLARE_SCRIPTABLE_OBJECT(MutationObserver);

  private: class Tracker;

  private: v8_glue::ScopedPersistent<v8::Function> callback_;
  private: std::unordered_map<Document*, Tracker*> tracker_map_;

  public: MutationObserver(v8::Handle<v8::Function> callback);
  public: virtual ~MutationObserver();

  public: void DidDeleteAt(Document* document, Posn offset, size_t length);
  public: void DidInsertAt(Document* document, Posn offset, size_t length);
  public: void DidMutateDocument(Document* document);
  public: void Disconnect();
  private: Tracker* GetTracker(Document* document) const;
  public: void Observe(Document* document,
                       const MutationObserverInit& options);
  public: std::vector<MutationRecord*> TakeRecords();

  DISALLOW_COPY_AND_ASSIGN(MutationObserver);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_mutation_observer_h)
