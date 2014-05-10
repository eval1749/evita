// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_composition_span_h)
#define INCLUDE_evita_dom_events_composition_span_h

#include "evita/v8_glue/scriptable.h"

namespace dom {

namespace bindings {
class CompositionSpanClass;
}

class CompositionSpan : public v8_glue::Scriptable<CompositionSpan> {
  DECLARE_SCRIPTABLE_OBJECT(CompositionSpan);
  friend class bindings::CompositionSpanClass;

  private: int data_;
  private: int end_;
  private: int start_;

  public: CompositionSpan(int start, int end, int data);
  public: ~CompositionSpan();

  public: int data() const { return data_; }
  public: void set_data(int new_data) { data_ = new_data; }
  public: int end() const { return end_; }
  public: void set_end(int new_end) { end_ = new_end; }
  public: int start() const { return start_; }
  public: void set_start(int new_start) { start_ = new_start; }

  DISALLOW_COPY_AND_ASSIGN(CompositionSpan);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_events_composition_span_h)
