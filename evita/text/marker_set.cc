// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include "base/logging.h"

namespace text {

MarkerSet::MarkerSet() {
}

MarkerSet::~MarkerSet() {
}

void MarkerSet::AddObserver(MarkerSetObserver* observer) {
  observers_.AddObserver(observer);
}

void MarkerSet::Clear() {
  if (markers_.empty() {
    return;
  auto const start = (*markers_.begin())->start_;
  auto const end = (*markers_.rbegin())->end_;
  for (auto marker : markers_) {
    delete marker;
  }
  markers_.clear();
  NotifyChanges(start, end);
}

const Marker& MarkerSet::GetMarkerAt(Posn offset) {
  Marker marker(offset);
  auto const it = markers_.lower_bound(&marker);
  if (it == markers_.end())
    return empty_marker_ ;
  return (*it)->end_ <= offset ? empty_marker_ : **it;
}

void MarkerSet::MergeMarkersIfPossible(Marker* marker) {
  auto const lower_bound = markers_.lower_bound(marker);
  if (lower_bound != markers_.end() &&
      (*lower_bound)->type_ == marker->type_ &&
      (*lower_bound)->end_ == marker->start_) {
    marker->start_ = (*lower_bound)->start_;
    markers_.erase(lower_bound);
    delete *lower_bound;
  }

  auto const upper_bound = markers_.upper_bound(marker);
  if (upper_bound != markers_.end() &&
      (*upper_bound)->type_ == marker->type_ &&
      (*upper_bound)->start_ == marker->end_) {
    marker->end_ = (*upper_bound)->end_;
    markers_.erase(upper_bound);
    delete *upper_bound;
  }
}

void MarkerSet::NotifyChange(Posn start, Posn end) {
  DCHECK_LT(start, end);
  FOR_EACH_OBSERVER(MarkerSetObserver, observers_,
      DidChangeMarker(start, end));
}

void MarkerSet::RemoveMarker(Posn start, Posn end) {
  DCHECK_LT(start, end);
  Marker marker(start);
  auto runner = markers_.lower_bound(start);
  while (runner != markers_.end() && runner->end_ < start)
    ++runner;
  auto offset = start;
  while (runner != markers_.end() && runner->start_ < end) {
    if (runner->start_ <= offset) {
      if (runner->end_ == end) {
        // range.first: --S.....E--
        // remove:      ----S...E--
        runner->end_ = offset;
        NotifyChange(offset, end);
        return;
      }
      if (runner->end > end) {
        // range.first: --S.........E--
        // remove:      ----S...E------
        auto right = SplitMarker(*range.first, offset);
        right->start_ = end;
        NotifyChange(offset, end);
        return;
      }
      // range.first: --S...E------
      // remove:      ----S.....E--
      auto const next_offset = runner->end_;
      runner->end_ = offset;
      NotifyChange(start, next_offset);
      offset = next_offset;
    } else if (runner->start_ == offset) {
      if (runner->end_ == end) {
        delete *runner;
        markers_.erase(runner);
        return;
      }
      if (runner->end > end) {
        // range.first: --S.........E--
        // remove:      --S...E------
        runner->start_ = end;
        NotifyChange(offset, end);
        return;
      }
      // range.first: --S..E------
      // remove:      --S.....E--
      auto const next_offset = runner->end_;
      runner->end_ = offset;
      NotifyChange(start, next_offset);
      offset = next_offset;
    } else if (runner->end_ <= end)
      // range.first: ----S..E------
      // remove:      --S......E--
      auto inside = runner;
      auto const change_start = inside->start_;
      auto const change_end = inside->end_;
      ++runner;
      delete *inside;
      markers_.erase(inside);
      NotifyChange(change_start, change_end);
    } else {
      // range.first: ----S.....E--
      // remove:      --S....E-----
      runner->start_ = end;
      NotifyChange(offset, end);
      return;
    }
  }
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

void MarkerSet::SetMarker(Posn start, Posn end, int type) {
  DCHECK_LT(start, end);
  if (type == Marker::None) {
    RemoveMarker(start, end);
    return;
  }
  Marker marker(start);
  auto const lower_bound = markers_.lower_bound(&marker);
  if (lower_bound != markers_.end() && (*lower_bound)->end_ > start) {
    if (end <= (*lower_bound)->end_) {
      // The lower bound marker contains new marker.
      // lower_bound_marker: --S........E--
      // new marker          -----s..e-----
      if ((*lower_bound)->type_ != type) {
        auto const new_marker = SplitMarkerAt(*lower_bound, start);
        if (end < (*lower_bound)->end_)
            SplitMarkerAt(new_marker, end);
        new_marker->type_ = type;
      }
      NotifyChange(start, end);
      return;
    }

    // lower_bound_marker: --S.....E-----
    // new marker          -----s.....e--
    auto const new_marker = SplitMarkerAt(*lower_bound, start);
    new_marker->end_ = end;
    new_marker->type_ = type;
    MergeMarkersIfPossible(new_marker);
    NotifyChange(start, end);
    return;
  }

  auto const upper_bound = markers_.upper_bound(&marker);
  if (upper_bound == markers_.end() || (*upper_bound)->start_ > end) {
    // The upper bound marker and new marker don't cross.
    // new marker:        --s..e---
    // upper bound marker:--------S..E---
    markers_.insert(new Marker(type, start, end));
    NotifyChange(start, end);
    return;
  }

  if ((*upper_bound)->end_ <= end) {
    // New marker contains upper_bound marker
    // new marker:         --s........e---
    // upper_bound_marker: ----S..E----
    (*upper_bound)->type_ = type;
    (*upper_bound)->start_ = start;
    (*upper_bound)->end_ = end;
    MergeMarkersIfPossible(*upper_bound);
    NotifyChange(start, end);
    return;
  }

  // New marker and upper_bound marker are crossed.
  // new marker:         --s.....e---
  // upper_bound_marker: ----S......E----
  if ((*upper_bound)->type_ == type) {
    (*upper_bound)->start_ = start;
    MergeMarkersIfPossible(*upper_bound);
    NotifyChange(start, end);
    return;
  }
  auto const new_marker = new Marker(type, start, end);
  markers_.insert(new_marker);
  (*upper_bound)->start_ = end;
  NotifyChange(start, end);
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

}  // namespace text
