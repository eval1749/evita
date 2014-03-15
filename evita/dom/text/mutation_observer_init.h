// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mutation_observer_init_h)
#define INCLUDE_evita_dom_mutation_observer_init_h

#include "evita/dom/converter.h"
#include "evita/dom/init_dict.h"

namespace dom {

class MutationObserverInit final : public InitDict {
  private: bool summary_;

  public: MutationObserverInit();
  public: virtual ~MutationObserverInit();

  public: bool summary() const { return summary_; }

  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(MutationObserverInit);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<dom::MutationObserverInit> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     dom::MutationObserverInit* out);
};
}  // namespace gin

#endif // !defined(INCLUDE_evita_dom_mutation_observer_init_h)
