// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "evita/text/offset.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// MarkerSet::ChangeScope
//
class MarkerSet::ChangeScope final {
 public:
  explicit ChangeScope(MarkerSetImpl* markers);
  ~ChangeScope();

  void Insert(Offset start, Offset end, const common::AtomicString& type);
  void Remove(Marker* marker);
  void Update(Marker* marker, Offset new_start, Offset new_end);

 private:
  struct InsertOperation {
    Offset start;
    Offset end;
    const common::AtomicString* type;
  };

  struct RemoveOperation final {
    Marker* marker;
  };

  struct UpdateOperation final {
    Marker* marker;
    Offset start;
    Offset end;
  };

  MarkerSetImpl* markers_;
  std::vector<InsertOperation> inserts_;
  std::vector<RemoveOperation> removes_;
  std::vector<UpdateOperation> updates_;

  DISALLOW_COPY_AND_ASSIGN(ChangeScope);
};

MarkerSet::ChangeScope::ChangeScope(MarkerSetImpl* markers)
    : markers_(markers) {}

MarkerSet::ChangeScope::~ChangeScope() {
  for (auto const remove : removes_) {
    markers_->erase(remove.marker);
    delete remove.marker;
  }
  for (auto const update : updates_) {
    update.marker->end_ = update.end;
    update.marker->start_ = update.start;
    DCHECK_LT(update.marker->start_, update.marker->end_);
  }
  for (auto const insert : inserts_) {
    markers_->insert(new Marker(insert.start, insert.end, *insert.type));
  }
}

void MarkerSet::ChangeScope::Insert(Offset start,
                                    Offset end,
                                    const common::AtomicString& type) {
  inserts_.push_back(InsertOperation{start, end, &type});
}

void MarkerSet::ChangeScope::Remove(Marker* marker) {
  removes_.push_back(RemoveOperation{marker});
}

void MarkerSet::ChangeScope::Update(Marker* marker,
                                    Offset new_start,
                                    Offset new_end) {
  if (marker->start_ == new_start && marker->end_ == new_end)
    return;
  if (new_start >= new_end) {
    Remove(marker);
    return;
  }
  updates_.push_back(UpdateOperation{marker, new_start, new_end});
}

//////////////////////////////////////////////////////////////////////
//
// MarkerSet
//
MarkerSet::MarkerSet(BufferMutationObservee* mutation_observee)
    : mutation_observee_(mutation_observee) {
  mutation_observee_->AddObserver(this);
}

MarkerSet::~MarkerSet() {
  mutation_observee_->RemoveObserver(this);
  Clear();
}

MarkerSet::MarkerSetImpl::iterator MarkerSet::lower_bound(Offset offset) {
  Marker marker(offset);
  return markers_.lower_bound(&marker);
}

void MarkerSet::AddObserver(MarkerSetObserver* observer) {
  observers_.AddObserver(observer);
}

void MarkerSet::Clear() {
  if (markers_.empty())
    return;
  // Remember minimum start offset and maximum end offset in this marker set
  // for notification.
  auto const start = (*markers_.begin())->start_;
  auto const end = (*markers_.rbegin())->end_;
  for (auto marker : markers_) {
    delete marker;
  }
  markers_.clear();
  NotifyChange(start, end);
}

const Marker* MarkerSet::GetMarkerAt(Offset offset) const {
  auto const marker = GetLowerBoundMarker(offset);
  return marker && marker->Contains(offset) ? marker : nullptr;
}

const Marker* MarkerSet::GetLowerBoundMarker(Offset offset) const {
  if (markers_.empty())
    return nullptr;
  Marker marker(offset + OffsetDelta(1));
  auto const present = markers_.lower_bound(&marker);
  if (present == markers_.end())
    return nullptr;
  DCHECK_LT(offset, (*present)->end_);
  return *present;
}

void MarkerSet::InsertMarker(Offset start,
                             Offset end,
                             const common::AtomicString& type) {
  DCHECK_LT(start, end);
  RemoveMarker(start, end);
  if (type.empty())
    return;

  auto const after = lower_bound(end);
  auto const can_merge_after = after != markers_.end() &&
                               (*after)->type_ == type &&
                               (*after)->start_ == end;
  auto const before = lower_bound(start);
  auto const can_merge_before = before != markers_.end() &&
                                (*before)->type_ == type &&
                                (*before)->end_ == start;

  ChangeScope change_scope(&markers_);
  NotifyChange(start, end);
  if (can_merge_after && can_merge_before) {
    (*after)->start_ = (*before)->start_;
    change_scope.Remove(*before);
    return;
  }
  if (can_merge_after) {
    (*after)->start_ = start;
    return;
  }
  if (can_merge_before) {
    (*before)->end_ = end;
    return;
  }
  markers_.insert(new Marker(start, end, type));
}

void MarkerSet::NotifyChange(Offset start, Offset end) {
  FOR_EACH_OBSERVER(MarkerSetObserver, observers_, DidChangeMarker(start, end));
}

void MarkerSet::RemoveMarker(Offset start, Offset end) {
  DCHECK_LT(start, end);
  if (markers_.empty())
    return;

  ChangeScope change_scope(&markers_);

  auto offset = start;
  auto runner = lower_bound(offset + OffsetDelta(1));
  while (offset < end) {
    if (runner == markers_.end() || (*runner)->start_ >= end)
      return;
    auto const marker = *runner;
    if (offset <= marker->start_ && marker->end_ <= end) {
      // marker: ----xxxx----
      // runner: --________--
      // after:  ------------
      NotifyChange(marker->start_, marker->end_);
      change_scope.Remove(marker);
    } else if (offset <= marker->start_) {
      // marker: ----xxxx----
      // runner: --____------
      // after:  ------xx----
      NotifyChange(marker->start_, end);
      change_scope.Update(marker, end, marker->end_);
    } else if (marker->end_ <= end) {
      // marker: --xxxx----
      // runner: ----____--
      // after:  --xx------
      NotifyChange(offset, marker->end_);
      change_scope.Update(marker, marker->start_, offset);
    } else {
      // marker: --xxxxxx--
      // runner: ----__----
      // after:  --xx--xx--
      NotifyChange(offset, end);
      change_scope.Insert(marker->start_, offset, marker->type_);
      change_scope.Update(marker, end, marker->end_);
    }
    offset = marker->end_;
    ++runner;
  }
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

// BufferMutationObserver
void MarkerSet::DidDeleteAt(Offset start, OffsetDelta length) {
  auto const end = start + length;
  ChangeScope change_scope(&markers_);
  for (auto runner = markers_.rbegin();
       runner != markers_.rend() && (*runner)->end_ > start; ++runner) {
    auto const marker = *runner;
    auto const marker_end = marker->end_ >= end ? marker->end_ - length : start;
    if (marker->start_ <= start) {
      change_scope.Update(marker, marker->start_, marker_end);
      continue;
    }
    auto const marker_start =
        marker->start_ >= end ? marker->start_ - length : start;
    change_scope.Update(marker, marker_start, marker_end);
  }
}

void MarkerSet::DidInsertBefore(Offset offset, OffsetDelta length) {
  auto const start = offset + OffsetDelta(1);
  for (auto runner = lower_bound(start); runner != markers_.end(); ++runner) {
    auto const marker = *runner;
    if (marker->start_ >= offset)
      marker->start_ = marker->start_ + length;
    marker->end_ = marker->end_ + length;
  }
}

}  // namespace text
