// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_MUTATION_OBSERVER_H_
#define EVITA_DOM_TEXT_MUTATION_OBSERVER_H_

#include <unordered_map>
#include <vector>

#include "evita/text/offset.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class TextDocument;
class MutationObserverInit;
class MutationRecord;

namespace bindings {
class MutationObserverClass;
}

class MutationObserver final : public v8_glue::Scriptable<MutationObserver> {
  DECLARE_SCRIPTABLE_OBJECT(MutationObserver);

 public:
  ~MutationObserver() final;

  void DidDeleteAt(TextDocument* document,
                   text::Offset offset,
                   text::OffsetDelta length);
  void DidInsertBefore(TextDocument* document,
                       text::Offset offset,
                       text::OffsetDelta length);
  void DidMutateTextDocument(TextDocument* document);

 private:
  friend class bindings::MutationObserverClass;
  class Tracker;

  explicit MutationObserver(v8::Handle<v8::Function> callback);

  void Disconnect();
  Tracker* GetTracker(TextDocument* document) const;
  void Observe(TextDocument* document, const MutationObserverInit& options);
  std::vector<MutationRecord*> TakeRecords();

  v8_glue::ScopedPersistent<v8::Function> callback_;
  std::unordered_map<TextDocument*, Tracker*> tracker_map_;

  DISALLOW_COPY_AND_ASSIGN(MutationObserver);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_MUTATION_OBSERVER_H_
