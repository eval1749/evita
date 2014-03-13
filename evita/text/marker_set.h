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

class MarkerSet;

class MarkerSet : public BufferMutationObserver {
  private: typedef std::set<Marker*> MarkerSetImpl;
  private: class ChangeScope;

  private: MarkerSetImpl markers_;
  private: BufferMutationObservee* const mutation_observee_;
  private: ObserverList<MarkerSetObserver> observers_;

  public: MarkerSet(BufferMutationObservee* provider);
  public: virtual ~MarkerSet();

  private: MarkerSetImpl::iterator lower_bound(Posn offset);

  public: void AddObserver(MarkerSetObserver* observer);
  public: void Clear();
  public: const Marker* GetMarkerAt(Posn offset) const;
  public: const Marker* GetLowerBoundMarker(Posn offset) const;
  private: void NotifyChange(Posn start, Posn end);
  public: void InsertMarker(Posn start, Posn end, int type);
  public: void RemoveMarker(Posn start, Posn end);
  public: void RemoveObserver(MarkerSetObserver* observer);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void DidInsertBefore(Posn offset, size_t length) override;

  // MarkerSetObserver

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_marker_set_h)
