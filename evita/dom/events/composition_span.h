// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_COMPOSITION_SPAN_H_
#define EVITA_DOM_EVENTS_COMPOSITION_SPAN_H_

#include "evita/ginx/scriptable.h"

namespace dom {

namespace bindings {
class CompositionSpanClass;
}

class CompositionSpan final : public ginx::Scriptable<CompositionSpan> {
  DECLARE_SCRIPTABLE_OBJECT(CompositionSpan);

 public:
  CompositionSpan(int start, int end, int data);
  ~CompositionSpan() final;

  int data() const { return data_; }
  void set_data(int new_data) { data_ = new_data; }
  int end() const { return end_; }
  void set_end(int new_end) { end_ = new_end; }
  int start() const { return start_; }
  void set_start(int new_start) { start_ = new_start; }

 private:
  friend class bindings::CompositionSpanClass;

  int data_;
  int end_;
  int start_;

  DISALLOW_COPY_AND_ASSIGN(CompositionSpan);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_COMPOSITION_SPAN_H_
