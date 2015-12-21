// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/marker.h"
#include "evita/text/offset.h"
#include "evita/text/static_range.h"

namespace text {

// Markers indexed by end offset
// See also: text::IntervalSet and text::LineNumberCache, they also manages
// ranges.
using Markers = std::map<Offset, std::unique_ptr<Marker>>;

namespace {

//////////////////////////////////////////////////////////////////////
//
// SimpleEditor
//
class SimpleEditor final {
 public:
  explicit SimpleEditor(Markers* markers);
  ~SimpleEditor() = default;

  Marker* Insert(Offset start, Offset end, const common::AtomicString& type);
  void InsertOrMerge(Offset start,
                     Offset end,
                     const common::AtomicString& type);
  void Remove(Marker* marker);
  // Splits |marker| to end with |offset| and returns newly created |Marker|
  // starts with |marker->start()| and ends with |offset|.
  Marker* Split(Marker* marker, Offset offset);

 private:
  Markers* const markers_;

  DISALLOW_COPY_AND_ASSIGN(SimpleEditor);
};

SimpleEditor::SimpleEditor(Markers* markers) : markers_(markers) {}

Marker* SimpleEditor::Insert(Offset start,
                             Offset end,
                             const common::AtomicString& type) {
  DCHECK_LT(start, end);
  DCHECK(!type.empty());
  DCHECK(markers_->find(end) == markers_->end()) << "Offset " << end
                                                 << " should not be in map.";
  auto marker_ptr = std::make_unique<Marker>(start, end, type);
  const auto marker = marker_ptr.get();
  const auto& result =
      markers_->insert(std::make_pair(end, std::move(marker_ptr)));
  DCHECK(result.second) << "Offset " << end << " should not be in map.";
  return marker;
}

// Note: There are no markers between |start| and |end|.
void SimpleEditor::InsertOrMerge(Offset start,
                                 Offset end,
                                 const common::AtomicString& type) {
  const auto& before = markers_->lower_bound(start);
  const auto can_merge_before = before != markers_->end() &&
                                before->second->end() == start &&
                                before->second->type() == type;
  const auto& after = markers_->lower_bound(end);
  const auto can_merge_after = after != markers_->end() &&
                               after->second->start() == end &&
                               before->second->type() == type;
  if (before != markers_->end() && after != markers_->end()) {
    DCHECK(before == after || std::next(before) == after)
        << "We should not have markers between " << start << " and " << end;
  }
  if (can_merge_before && can_merge_after) {
    Marker::Editor(after->second.get()).SetStart(before->second->start());
    Remove(before->second.get());
    return;
  }
  if (can_merge_before) {
    const auto before_start = before->second->start();
    Remove(before->second.get());
    Insert(before_start, end, type);
    return;
  }
  if (can_merge_after) {
    Marker::Editor(after->second.get()).SetStart(before->second->start());
    return;
  }
  Insert(start, end, type);
}

void SimpleEditor::Remove(Marker* marker) {
  const auto& it = markers_->find(marker->end());
  DCHECK(it != markers_->end()) << *marker << " should be in map.";
  markers_->erase(it);
}

Marker* SimpleEditor::Split(Marker* marker, Offset offset) {
  DCHECK_GT(offset, marker->start());
  DCHECK_LT(offset, marker->end());
  const auto new_marker_start = marker->start();
  Marker::Editor(marker).SetStart(offset);
  return Insert(new_marker_start, offset, marker->type());
}

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class Editor final {
 public:
  explicit Editor(Markers* markers);
  ~Editor();

  void Insert(Offset start, Offset end, const common::AtomicString& type);
  void Remove(Marker* marker);
  void Update(Marker* marker, Offset new_start, Offset new_end);

 private:
  struct InsertOperation {
    Offset start;
    Offset end;
    common::AtomicString type;
  };

  struct RemoveOperation final {
    Marker* marker;
  };

  struct UpdateOperation final {
    Marker* marker;
    Offset start;
  };

  Markers* markers_;
  std::vector<InsertOperation> inserts_;
  std::vector<RemoveOperation> removes_;
  std::vector<UpdateOperation> updates_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

Editor::Editor(Markers* markers) : markers_(markers) {}

Editor::~Editor() {
  SimpleEditor editor(markers_);
  for (const auto remove : removes_)
    editor.Remove(remove.marker);
  for (const auto update : updates_)
    Marker::Editor(update.marker).SetStart(update.start);
  for (const auto insert : inserts_)
    editor.Insert(insert.start, insert.end, insert.type);
}

void Editor::Insert(Offset start,
                    Offset end,
                    const common::AtomicString& type) {
  inserts_.push_back(InsertOperation{start, end, type});
}

void Editor::Remove(Marker* marker) {
  removes_.push_back(RemoveOperation{marker});
}

void Editor::Update(Marker* marker, Offset new_start, Offset new_end) {
  if (marker->start() == new_start && marker->end() == new_end)
    return;
  if (new_start >= new_end)
    return Remove(marker);
  if (marker->end() == new_end)
    return updates_.push_back(UpdateOperation{marker, new_start});
  Remove(marker);
  Insert(new_start, new_end, marker->type());
}

//////////////////////////////////////////////////////////////////////
//
// Notifier
//
class Notifier final {
 public:
  explicit Notifier(base::ObserverList<MarkerSetObserver>* observers);
  ~Notifier();

  void NotifyChange(Offset start, Offset end);

 private:
  std::vector<std::pair<Offset, Offset>> changes_;
  base::ObserverList<MarkerSetObserver>* const observers_;

  DISALLOW_COPY_AND_ASSIGN(Notifier);
};

Notifier::Notifier(base::ObserverList<MarkerSetObserver>* observers)
    : observers_(observers) {}

Notifier::~Notifier() {
  for (const auto& change : changes_) {
    FOR_EACH_OBSERVER(MarkerSetObserver, *observers_,
                      DidChangeMarker(change.first, change.second));
  }
}

void Notifier::NotifyChange(Offset start, Offset end) {
  if (!changes_.empty() && changes_.back().second == start) {
    changes_.back().second = end;
    return;
  }
  changes_.emplace_back(start, end);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MarkerSet::Impl
//
class MarkerSet::Impl final : public BufferMutationObserver {
 public:
  explicit Impl(const Buffer& buffer);
  ~Impl() final;

  void AddObserver(MarkerSetObserver* observer);
  const Marker* GetMarkerAt(Offset offset) const;
  const Marker* GetLowerBoundMarker(Offset offset) const;
  void InsertMarker(const StaticRange& range, const common::AtomicString& type);
  void RemoveObserver(MarkerSetObserver* observer);

 private:
  // BufferMutationObserver
  void DidDeleteAt(const StaticRange& range) final;
  void DidInsertBefore(const StaticRange& range) final;

  Markers markers_;
  const Buffer& buffer_;
  base::ObserverList<MarkerSetObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

MarkerSet::Impl::Impl(const Buffer& buffer)
    : buffer_(buffer) {
  buffer_.AddObserver(this);
}

MarkerSet::Impl::~Impl() {
  buffer_.RemoveObserver(this);
  markers_.clear();
}

void MarkerSet::Impl::AddObserver(MarkerSetObserver* observer) {
  observers_.AddObserver(observer);
}

const Marker* MarkerSet::Impl::GetMarkerAt(Offset offset) const {
  const auto marker = GetLowerBoundMarker(offset);
  return marker && marker->Contains(offset) ? marker : nullptr;
}

const Marker* MarkerSet::Impl::GetLowerBoundMarker(Offset offset) const {
  const auto& it = markers_.lower_bound(offset);
  if (it == markers_.end())
    return nullptr;
  const auto marker = it->second.get();
  if (offset < marker->end())
    return marker;
  const auto& next_it = std::next(it);
  if (next_it == markers_.end())
    return nullptr;
  return next_it->second.get();
}

void MarkerSet::Impl::InsertMarker(const StaticRange& range,
                                   const common::AtomicString& type) {
  const auto start = range.start();
  const auto end = range.end();

  SimpleEditor editor(&markers_);
  Notifier notifier(&observers_);

  // Step 1: Collect markers in range; we'll remove them
  std::vector<Marker*> markers;
  for (auto it = markers_.lower_bound(start); it != markers_.end(); ++it) {
    const auto marker = it->second.get();
    if (marker->start() >= end)
      break;
    if (marker->end() == start)
      continue;
    markers.push_back(marker);
  }

  if (markers.empty()) {
    // There are no markers between start/end.
    if (type.empty())
      return;
    editor.InsertOrMerge(start, end, type);
    notifier.NotifyChange(start, end);
    return;
  }

  const auto first_marker = markers.front();
  if (markers.size() == 1) {
    if (first_marker->start() == start && end == first_marker->end()) {
      // |first_marker| equals to start/end
      if (first_marker->type() == type)
        return;
      notifier.NotifyChange(first_marker->start(), first_marker->end());
      if (type.empty())
        editor.Remove(first_marker);
      else
        Marker::Editor(first_marker).SetType(type);
      return;
    }
    if (first_marker->type() == type && first_marker->start() <= start &&
        end <= first_marker->end()) {
      // |first_marker| contains start/end
      return;
    }
  }

  // Step 2: Adjust first collected marker
  if (first_marker->start() < start)
    editor.Split(markers.front(), start);
  // Step 3: Adjust last collected marker
  if (markers.back()->end() > end)
    markers.back() = editor.Split(markers.back(), end);

  // Step 4-1: Remove markers and collect changes
  if (type.empty()) {
    for (const auto marker : markers) {
      notifier.NotifyChange(marker->start(), marker->end());
      editor.Remove(marker);
    }
    return;
  }

  // Step 4-2: Remove marker and collect changes
  auto offset = start;
  for (const auto marker : markers) {
    if (offset < marker->start())
      notifier.NotifyChange(offset, marker->start());
    if (marker->type() != type)
      notifier.NotifyChange(offset, marker->end());
    offset = marker->end();
    editor.Remove(marker);
  }
  if (offset < end)
    notifier.NotifyChange(offset, end);

  // Step 5: Insert new marker
  editor.InsertOrMerge(start, end, type);
}

void MarkerSet::Impl::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

// BufferMutationObserver
void MarkerSet::Impl::DidDeleteAt(const StaticRange& range) {
  const auto length = range.length();
  Editor editor(&markers_);
  for (auto it = markers_.rbegin(); it != markers_.rend(); ++it) {
    const auto marker = it->second.get();
    if (marker->end() <= range.start())
      break;
    const auto marker_end =
        marker->end() > range.end() ? marker->end() - length : range.start();
    if (marker->start() <= range.start()) {
      editor.Update(marker, marker->start(), marker_end);
      continue;
    }
    if (marker->start() < range.end()) {
      editor.Update(marker, range.start(), marker_end);
      continue;
    }
    editor.Update(marker, marker->start() - length, marker_end);
  }
}

void MarkerSet::Impl::DidInsertBefore(const StaticRange& range) {
  const auto length = range.length();
  Editor editor(&markers_);
  for (auto it = markers_.rbegin(); it != markers_.rend(); ++it) {
    const auto marker = it->second.get();
    if (marker->end() < range.start())
      break;
    const auto marker_end = marker->end() + length;
    if (marker->start() < range.start()) {
      editor.Update(marker, marker->start(), marker_end);
      continue;
    }
    editor.Update(marker, marker->start() + length, marker_end);
  }
}

//////////////////////////////////////////////////////////////////////
//
// MarkSet
MarkerSet::MarkerSet(const Buffer& buffer)
    : impl_(new Impl(buffer)) {}

MarkerSet::~MarkerSet() {}

void MarkerSet::AddObserver(MarkerSetObserver* observer) {
  impl_->AddObserver(observer);
}

const Marker* MarkerSet::GetMarkerAt(Offset offset) const {
  return impl_->GetMarkerAt(offset);
}

const Marker* MarkerSet::GetLowerBoundMarker(Offset offset) const {
  return impl_->GetLowerBoundMarker(offset);
}

// Insert marker from |start| to |end|, exclusive.
void MarkerSet::InsertMarker(const StaticRange& range,
                             const common::AtomicString& type) {
  impl_->InsertMarker(range, type);
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  impl_->RemoveObserver(observer);
}

}  // namespace text
