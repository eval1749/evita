// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_MARKER_SET_H_
#define EVITA_TEXT_MARKER_SET_H_

#include <set>

#include "base/basictypes.h"
#include "base/observer_list.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set_observer.h"
#include "evita/text/offset.h"

namespace common {
class AtomicString;
}

namespace text {

class MarkerSet final : public BufferMutationObserver {
 public:
  explicit MarkerSet(BufferMutationObservee* provider);
  ~MarkerSet() final;

  // Remove |observer|
  void AddObserver(MarkerSetObserver* observer);

  // Get marker at |offset|.
  const Marker* GetMarkerAt(Offset offset) const;

  // Get marker starting at |offset| or after |offset|. This function is
  // provided for reducing call for |GetMarkerAt()| on every position in
  // document. See |TextFormatter::TextScanner::spelling()|.
  const Marker* GetLowerBoundMarker(Offset offset) const;

  // Insert marker from |start| to |end|, exclusive.
  void InsertMarker(Offset start, Offset end, const common::AtomicString& type);

  void RemoveMarkerForTesting(Offset start, Offset end) {
    RemoveMarker(start, end);
  }

  // Remove |observer|
  void RemoveObserver(MarkerSetObserver* observer);

 private:
  using MarkerSetImpl = std::set<Marker*>;
  class ChangeScope;

  MarkerSetImpl::iterator lower_bound(Offset offset);

  // Remove all markers in this |MarkerSet|.
  void Clear();

  // Notify marker changes to observers.
  void NotifyChange(Offset start, Offset end);

  // Remove marker from |start| to |end|, exclusive.
  void RemoveMarker(Offset start, Offset end);

  // BufferMutationObserver
  void DidDeleteAt(Offset offset, OffsetDelta length) final;
  void DidInsertAt(Offset offset, OffsetDelta length) final;
  void DidInsertBefore(Offset offset, OffsetDelta length) final;

  MarkerSetImpl markers_;
  BufferMutationObservee* const mutation_observee_;
  base::ObserverList<MarkerSetObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif  // EVITA_TEXT_MARKER_SET_H_
