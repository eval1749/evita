// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include <vector>

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// MarkerSet::ChangeScope
//
class MarkerSet::ChangeScope {
  private: MarkerSetImpl* markers_;
  private: std::vector<Marker*> markers_to_remove_;

  public: ChangeScope(MarkerSetImpl* markers);
  public: ~ChangeScope();

  public: void Remove(Marker* marker);

  DISALLOW_COPY_AND_ASSIGN(ChangeScope);
};

MarkerSet::ChangeScope::ChangeScope(MarkerSetImpl* markers)
    : markers_(markers) {
}

MarkerSet::ChangeScope::~ChangeScope() {
  for (auto const marker : markers_to_remove_) {
    markers_->erase(marker);
    delete marker;
  }
}

void MarkerSet::ChangeScope::Remove(Marker* marker) {
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
  auto const present = lower_bound(offset + 1);
  if (present == markers_.end())
    return Marker();
  DCHECK_LT(offset, (*present)->end_);
  return offset >= (*present)->start_ ? **present : Marker();
}

void MarkerSet::InsertMarker(Posn start, Posn end, int type) {
  DCHECK_LT(start, end);

  auto offset = start;
  while (offset < end) {
    auto iterator = lower_bound(offset);
    if (iterator == markers_.end() || (*iterator)->start_ >= end)
      break;

    auto const present = *iterator;
    if (present->end_ >= end) {
      if (present->type_ == type) {
        if (present->start_ > start) {
          // present: ----nnnn--
          // insert:  --nnnn----
          NotifyChange(start, present->start_);
          present->start_ = start;
        }
        return;
      }
      if (present->start_ > start) {
        // present: ----pppp--
        // insert:  --nnnn----
        // after:   --nnnn----
        present->start_ = end;
        break;
      }
      // present: --pppppppp--
      // insert:  ----nnnn----
      // after:   --ppnnnnpp--
      SplitMarker(present, offset, end);
      break;
    }

   ++iterator;
   if (iterator == markers_.end() || (*iterator)->start_ >= end) {
     // |present| is the last marker.
     if (present->type_ == type) {
        if (present->start_ > offset) {
          NotifyChange(offset, present->start_);
          present->start_ = offset;
        }
        NotifyChange(present->end_, end);
        present->end_ = end;
        return;
     }
     if (present->start_ >= offset) {
       NotifyChange(offset, end);
       present->start_ = offset;
       present->end_ = end;
       present->type_ = type;
       return;
     }
     // present: --pppp----
     // insert:  ----nnnn--
     // after:   --pp????--
     present->end_ = offset;
     break;
   }

    auto const following = *iterator;
    if (following->end_ <= end && following->type_ != type) {
      NotifyChange(following->start_, following->end_);
      following->type_ = type;
    }

    if (present->type_ == type) {
      // present:   ---nnnn-----
      // following: -------ffff--
      // insert:    --nnnnnnn----
      // after:     --nnnnnffff--

      // present:   --nnnnnn------
      // following: --------ffff--
      // insert:    ------nnnn----
      // after:     --nnnnnnffff--
      if (present->start_ > offset)
        present->start_ = offset;
      present->end_ = following->start_;
      TryMerge(present, following);
      offset = present->end_;
      continue;
    }

    if (present->start_ >= offset) {
      // present:   ----pppp--------
      // following: ----------ffff--
      // insert:    --nnnnnnnnnn??--
      // after:     --nnnnnnnnnn??--
      NotifyChange(offset, following->start_);
      present->start_ = offset;
      present->end_ = following->start_;
      present->type_ = type;
      TryMerge(present, following);
      offset = present->end_;
      continue;
    }

    present->end_ = offset;
    if (following->type_ == type) {
      // present:   --pppp--------
      // insert:    ------nnnn----
      // following: --------nnnn--
      // after:     --ppppnnnnnn--
      if (offset != following->start_) {
        NotifyChange(offset, following->start_);
        following->start_ = present->end_;
      }
      offset = following->end_;
    } else if (following->end_ <= end) {
      NotifyChange(offset, following->end_);
      following->start_ = offset;
      offset = following->end_;
    } else {
      following->start_ = end;
      break;
    }
  }

  if (offset < end) {
    NotifyChange(offset, end);
    markers_.insert(new Marker(offset, end, type));
  }
}

void MarkerSet::NotifyChange(Posn start, Posn end) {
  DCHECK_LT(start, end);
  FOR_EACH_OBSERVER(MarkerSetObserver, observers_,
      DidChangeMarker(start, end));
}

void MarkerSet::RemoveMarker(Posn start, Posn end) {
  DCHECK_LT(start, end);
  if (markers_.empty())
    return;

  ChangeScope change_scope(&markers_);

  auto offset = start;
  auto runner = lower_bound(offset + 1);
  while (offset < end) {
    if (runner == markers_.end() || (*runner)->start_ >= end)
      return;

    auto const runner_end = (*runner)->end_;
    auto const runner_start = (*runner)->start_;
    auto const runner_type = (*runner)->type_;

    if (runner_end >= end) {
      if (runner_start <= offset) {
        // before: --xxxxxxxx--
        // new:    ----____----
        // after:  --xx____xx--
        NotifyChange(offset, end);
        (*runner)->start_ = end;
        if (runner_start < offset)
          markers_.insert(new Marker(runner_start, offset, runner_type));
        return;
      }

      // before: ----xxxx--
      // new:    --____----
      // after:  ----__xx--
      if (runner_end == end)
        change_scope.Remove(*runner);
      else
        (*runner)->start_ = end;
      NotifyChange(start, end);
      return;
    }

    if (runner_start < offset) {
      // before: --xxxxx??--
      // new:    ----_____--
      // after:  --xx___??--
      NotifyChange(offset, runner_end);
      (*runner)->end_ = offset;
    } else if (runner_start == offset) {
      // before: --xxxx??--
      // new:    --_____--
      // after:  --____??--
      NotifyChange(offset, runner_end);
      change_scope.Remove(*runner);
    } else {
      // before: ----xxxx??--
      // new:    --________--
      // after:  ----____??--
      NotifyChange(offset, runner_end);
      change_scope.Remove(*runner);
    }

    offset = (*runner)->end_;
    ++runner;
  }
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

// Split specified marker into atmost two markers by excluding range start,
// end(exclusive).
void MarkerSet::SplitMarker(Marker* marker, Posn start, Posn end) {
  DCHECK_LT(start, end);
  DCHECK_LE(marker->start_, start);
  DCHECK_GE(marker->end_, end);

  if (marker->start_ == start && marker->end_ == end) {
    markers_.erase(marker);
    delete marker;
    return;
  }

  if (marker->start_ == start) {
    marker->start_ = end;
    return;
  }

  auto const marker_end = marker->end_;
  marker->end_ = start;
  if (end < marker_end)
    markers_.insert(new Marker(end, marker_end, marker->type_));
}

void MarkerSet::TryMerge(Marker* marker, Marker* following) {
  if (!following || marker->type_ != following->type_ ||
      marker->end_ != following->start_) {
    return;
  }
  markers_.erase(following);
  marker->end_ = following->end_;
  delete following;
}

// BufferMutationObserver
void MarkerSet::DidDeleteAt(Posn offset, size_t length) {
  ChangeScope change_scope(&markers_);
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ > offset) {
      marker->start_ = std::max(static_cast<Posn>(marker->start_ - length),
                                offset);
    }
    if (marker->end_ > offset) {
      marker->end_ = std::max(static_cast<Posn>(marker->start_ - length),
                              offset);
    }
    if (marker->start_ == marker->end_)
      change_scope.Remove(marker);
  }
}

void MarkerSet::DidInsertAt(Posn offset, size_t length) {
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ > offset)
      marker->start_ += length;
    marker->end_ += length;
  }
}

void MarkerSet::DidInsertBefore(Posn offset, size_t length) {
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ >= offset)
      marker->start_ += length;
    marker->end_ += length;
  }
}

}  // namespace text
