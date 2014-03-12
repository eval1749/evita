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
  private: class ChangeMarkerScope;

  private: MarkerSetImpl markers_;
  private: ObserverList<MarkerSetObserver> observers_;

  public: MarkerSet();
  public: virtual ~MarkerSet();

  private: MarkerSetImpl::iterator lower_bound(Posn offset);
  private: MarkerSetImpl::iterator upper_bound(Posn offset);

  public: void AddObserver(MarkerSetObserver* observer);
  public: void Clear();
  public: Marker GetMarkerAt(Posn offset);
  private: void NotifyChange(Posn start, Posn end);
  public: void InsertMarker(Posn start, Posn end, int type);
  public: void RemoveMarker(Posn start, Posn end);
  private: bool RemoveMarkerImpl(Posn start, Posn end,
                                 const MarkerSetImpl::iterator& iterator);
  public: void RemoveObserver(MarkerSetObserver* observer);
  private: Marker* SplitMarkerAt(Marker* marker, Posn offset);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  // MarkerSetObserver

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_marker_set_h)
