// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ChangeMarkerScope
//
class MarkerSet::ChangeMarkerScope {
  private: MarkerSetImpl* markers_;
  private: std::vector<Marker*> markers_to_remove_;

  public: ChangeMarkerScope(MarkerSetImpl* markers);
  public: ~ChangeMarkerScope();

  public: void Remove(Marker* marker);

  DISALLOW_COPY_AND_ASSIGN(ChangeMarkerScope);
};

MarkerSet::ChangeMarkerScope::ChangeMarkerScope(MarkerSetImpl* markers)
    : markers_(markers) {
}

MarkerSet::ChangeMarkerScope::~ChangeMarkerScope() {
  for (auto marker : markers_to_remove_) {
    markers_->erase(marker);
  }
}

void MarkerSet::ChangeMarkerScope::Remove(Marker* marker) {
  markers_to_remove_.push_back(marker);
}

//////////////////////////////////////////////////////////////////////
//
// MarkerSet
//
MarkerSet::MarkerSet() {
}

MarkerSet::~MarkerSet() {
}

MarkerSet::MarkerSetImpl::iterator MarkerSet::lower_bound(Posn offset) {
  Marker marker(offset);
  return markers_.lower_bound(&marker);
}

MarkerSet::MarkerSetImpl::iterator MarkerSet::upper_bound(Posn offset) {
  Marker marker(offset);
  return markers_.upper_bound(&marker);
}

void MarkerSet::AddObserver(MarkerSetObserver* observer) {
  observers_.AddObserver(observer);
}

void MarkerSet::Clear() {
  if (markers_.empty())
    return;
  auto const start = (*markers_.begin())->start_;
  auto const end = (*markers_.rbegin())->end_;
  for (auto marker : markers_) {
    delete marker;
  }
  markers_.clear();
  NotifyChange(start, end);
}

Marker MarkerSet::GetMarkerAt(Posn offset) {
  if (markers_.empty())
    return Marker();
  auto const left = lower_bound(offset);
  if (left != markers_.end()) {
    return offset < (*left)->start_ || offset >= (*left)->end_ ?
        Marker() : **left;
  }
  auto const right = markers_.rbegin();
  return offset < (*right)->start_ || offset >= (*right)->end_ ?
        Marker() : **right;
}

void MarkerSet::InsertMarker(Posn start, Posn end, int type) {
  DCHECK_LT(start, end);
  RemoveMarker(start, end);
  if (type == Marker::None)
    return;
  NotifyChange(start, end);

  auto const left = lower_bound(start);
  auto const right = upper_bound(end);
  if (left != markers_.end() && (*left)->type_ == type &&
      (*left)->end_ == start) {
    if (right != markers_.end() && (*right)->type_ == type &&
        (*right)->start_ == end) {
      (*left)->end_ = (*right)->end_;
      markers_.erase(right);
      delete *right;
      return;
    }
    (*left)->end_ = end;
    return;
  }

  if (right != markers_.end() && (*right)->type_ == type &&
      (*right)->start_ == end) {
    (*right)->start_ = start;
    return;
  }

  markers_.insert(new Marker(type, start, end));
}

void MarkerSet::RemoveMarker(Posn start, Posn end) {
  DCHECK_LT(start, end);
  if (markers_.empty())
    return;
  if (RemoveMarkerImpl(start, end, lower_bound(start)))
    return;
  if (RemoveMarkerImpl(start, end, upper_bound(start)))
    return;
  if (RemoveMarkerImpl(start, end, lower_bound(end)))
    return;
  if (RemoveMarkerImpl(start, end, upper_bound(end)))
    return;
}

bool MarkerSet::RemoveMarkerImpl(Posn start, Posn end,
                                 const MarkerSetImpl::iterator& iterator) {
  DCHECK_LT(start, end);
  if (iterator == markers_.end())
    return false;

  ChangeMarkerScope change_marker_scope(&markers_);

  auto runner = iterator;
  while (runner != markers_.end() && (*runner)->end_ < start) {
    ++runner;
  }

  while (runner != markers_.end() && (*runner)->end_ < end) {
    if ((*runner)->start_ < start) {
      // before: --ooooooo-----
      // remove: -----xxxxxxx--
      // after:  --ooo--------
      NotifyChange(start, (*runner)->end_);
      (*runner)->end_ = start;
    } else {
      // before: ----oooo-----
      // remove: --xxxxxxxx---
      // after:  -------------
      NotifyChange((*runner)->start_, (*runner)->end_);
      change_marker_scope.Remove(*runner);
    }
    ++runner;
  }

  if (runner == markers_.end())
    return true;

  if ((*runner)->end_ == end) {
    if ((*runner)->start_ < start) {
      // before: --oooooooo---
      // remove: ------xxxx--
      // after:  --oooo---
      NotifyChange(start, end);
      (*runner)->end_ = start;
      return true;
    }
    // before: -----oooo--
    // remove: --xxxxxxx--
    // after:  -----------
    NotifyChange((*runner)->start_, end);
    change_marker_scope.Remove(*runner);
    return true;
  }

  if ((*runner)->start_ < start) {
    // before: --ooooooooo--
    // remove: ----xxxxx----
    // after:  --oo-----oo--
    NotifyChange(start, end);
    auto const right = SplitMarkerAt(*runner, start);
    right->start_ = end;
    return true;
  }

  // before: ----oooooo--
  // remove: --xxxxx-----
  // after:  -------ooo--
  NotifyChange((*runner)->start_, end);
  (*runner)->start_ = end;
  return true;
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

Marker* MarkerSet::SplitMarkerAt(Marker* marker, Posn offset) {
  DCHECK_GT(offset, marker->start_);
  DCHECK_LT(offset, marker->end_);
  auto const new_marker = new Marker(marker->type_, offset, marker->end_);
  markers_.insert(new_marker);
  return new_marker;
}

// BufferMutationObserver
void MarkerSet::DidDeleteAt(Posn offset, size_t length) {
  for (auto* marker : markers_) {
    if (marker->start_ > offset) {
      marker->start_ = std::max(static_cast<Posn>(marker->start_ - length),
                                offset);
    }
    if (marker->end_ > offset) {
      marker->end_ = std::max(static_cast<Posn>(marker->end_ - length),
                              offset);
    }
  }
}

void MarkerSet::DidInsertAt(Posn offset, size_t length) {
  for (auto* marker : markers_) {
    if (marker->start_ > offset)
      marker->start_ += length;
    if (marker->end_ > offset)
      marker->end_ += length;
  }
}

// MarkerSetObserver
void MarkerSet::NotifyChange(Posn start, Posn end) {
  DCHECK_LT(start, end);
  FOR_EACH_OBSERVER(MarkerSetObserver, observers_,
      DidChangeMarker(start, end));
}

}  // namespace text
