// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_
#define EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_

#include <memory>
#include <vector>

#include "evita/ginx/scoped_persistent.h"
#include "evita/ginx/scriptable.h"
#include "evita/text/models/offset.h"

namespace dom {

class ScriptHost;
class TextDocument;
class TextMutationObserverInit;
class TextMutationRecord;

namespace bindings {
class TextMutationObserverClass;
}

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver implements IDL interface |TextMutationObserver|.
//
class TextMutationObserver final
    : public ginx::Scriptable<TextMutationObserver> {
  DECLARE_SCRIPTABLE_OBJECT(TextMutationObserver);

 public:
  class Tracker;

  ~TextMutationObserver() final;

 private:
  friend class bindings::TextMutationObserverClass;

  TextMutationObserver(ScriptHost* script_host,
                       v8::Local<v8::Function> callback);

  // Bindings
  void Disconnect();
  void Observe(TextDocument* document, const TextMutationObserverInit& options);
  std::vector<TextMutationRecord*> TakeRecords();

  ginx::ScopedPersistent<v8::Function> callback_;
  std::vector<std::unique_ptr<Tracker>> trackers_;

  DISALLOW_COPY_AND_ASSIGN(TextMutationObserver);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_MUTATION_OBSERVER_H_
