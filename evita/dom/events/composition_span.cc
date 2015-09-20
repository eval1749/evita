// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/composition_span.h"

namespace dom {

CompositionSpan::CompositionSpan(int start, int end, int data)
    : data_(data), end_(end), start_(start) {}

CompositionSpan::~CompositionSpan() {}

}  // namespace dom
