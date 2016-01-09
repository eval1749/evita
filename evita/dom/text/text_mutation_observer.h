// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_
#define EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_

#include <unordered_map>
#include <vector>

#include "evita/text/offset.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class TextDocument;
class TextMutationObserverInit;
class TextMutationRecord;

namespace bindings {
class TextMutationObserverClass;
}

class TextMutationObserver final
    : public v8_glue::Scriptable<TextMutationObserver> {
  DECLARE_SCRIPTABLE_OBJECT(TextMutationObserver);

 public:
  ~TextMutationObserver() final;

  void DidDeleteAt(TextDocument* document,
                   text::Offset offset,
                   text::OffsetDelta length);
  void DidInsertBefore(TextDocument* document,
                       text::Offset offset,
                       text::OffsetDelta length);
  void DidMutateTextDocument(TextDocument* document);

 private:
  friend class bindings::TextMutationObserverClass;
  class Tracker;

  explicit TextMutationObserver(v8::Handle<v8::Function> callback);

  void Disconnect();
  Tracker* GetTracker(TextDocument* document) const;
  void Observe(TextDocument* document, const TextMutationObserverInit& options);
  std::vector<TextMutationRecord*> TakeRecords();

  v8_glue::ScopedPersistent<v8::Function> callback_;
  std::unordered_map<TextDocument*, Tracker*> tracker_map_;

  DISALLOW_COPY_AND_ASSIGN(TextMutationObserver);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_
