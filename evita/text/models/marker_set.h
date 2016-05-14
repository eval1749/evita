// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_MODELS_MARKER_SET_H_
#define EVITA_TEXT_MODELS_MARKER_SET_H_

#include <memory>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/text/models/marker_set_observer.h"
#include "evita/text/models/offset.h"

namespace base {
class AtomicString;
}

namespace text {

class Buffer;
class Marker;
class StaticRange;

//////////////////////////////////////////////////////////////////////
//
// MarkerSet
//
class MarkerSet final {
 public:
  explicit MarkerSet(const Buffer& buffer);
  ~MarkerSet();

  const Buffer& buffer() const;

  // Add |observer|
  void AddObserver(MarkerSetObserver* observer) const;

  // Get marker at |offset|.
  const Marker* GetMarkerAt(Offset offset) const;

  // Get marker starting at |offset| or after |offset|. This function is
  // provided for reducing call for |GetMarkerAt()| on every position in
  // document. See |TextFormatter::TextScanner::spelling()|.
  const Marker* GetLowerBoundMarker(Offset offset) const;

  // Insert marker to |range| with |type|.
  void InsertMarker(const StaticRange& range, base::AtomicString type);

  // Remove |observer|
  void RemoveObserver(MarkerSetObserver* observer) const;

 private:
  class Impl;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif  // EVITA_TEXT_MODELS_MARKER_SET_H_
