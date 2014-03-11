// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_set_h)
#define INCLUDE_evita_text_marker_set_h

#include <set>

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set_observer.h"

namespace text {

class MarkerSet : public BufferMutationObserver {
  private: Marker empty_marker_;
  private: std::set<Marker*> markers_;
  private: ObserverList<MarkerSetObserver> observers_;

  public: MarkerSet();
  public: virtual ~MarkerSet();

  public: void AddObserver(MarkerSetObserver* observer);
  public: void Clear();
  public: const Marker& GetMarkerAt(Posn offset);
  private: void MergeMarkersIfPossible(Marker* marker);
  private: void NotifyChange(Posn start, Posn end);
  public: void RemoveMarker(Posn start, Posn end);
  public: void RemoveObserver(MarkerSetObserver* observer);
  public: void SetMarker(Posn start, Posn end, int type);
  private: Marker* SplitMarkerAt(Marker* marker, Posn offset);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_marker_set_h)
