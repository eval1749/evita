// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mutation_observer_init_h)
#define INCLUDE_evita_dom_mutation_observer_init_h

#include "evita/dom/dictionary.h"

namespace dom {

class MutationObserverInit final : public Dictionary {
  private: bool summary_;

  public: MutationObserverInit();
  public: virtual ~MutationObserverInit();

  public: bool summary() const { return summary_; }

  // dom::Dictionary
  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_mutation_observer_init_h)
