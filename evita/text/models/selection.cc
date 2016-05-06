// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/models/selection.h"

#include "base/observer_list.h"
#include "base/trace_event/trace_event.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/range.h"
#include "evita/text/models/selection_change_observer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Model
//
class Selection::Model final : public Range {
 public:
  explicit Model(const Selection& selection, const Range* range);
  ~Model() final;

  void AddObserver(SelectionChangeObserver* observer);
  bool IsStartActive() const { return start_is_active_; }
  void RemoveObserver(SelectionChangeObserver* observer);
  void SetStartIsActive(bool new_start_is_active);

 private:
  void NotifyChange();

  // Range
  void DidChangeRange() final;

  base::ObserverList<SelectionChangeObserver> observers_;
  // Used only for trace logging.
  const Selection& selection_;
  bool start_is_active_;

  DISALLOW_COPY_AND_ASSIGN(Model);
};

Selection::Model::Model(const Selection& selection, const Range* range)
    : Range(range->buffer(), range->start(), range->end()),
      selection_(selection),
      start_is_active_(false) {}

Selection::Model::~Model() {}

void Selection::Model::AddObserver(SelectionChangeObserver* observer) {
  observers_.AddObserver(observer);
}

void Selection::Model::DidChangeRange() {
  NotifyChange();
}

void Selection::Model::NotifyChange() {
  TRACE_EVENT_WITH_FLOW0("views", "Selection::NotifyChange", &selection_,
                         TRACE_EVENT_FLAG_FLOW_OUT);
  FOR_EACH_OBSERVER(SelectionChangeObserver, observers_, DidChangeSelection());
}

void Selection::Model::RemoveObserver(SelectionChangeObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Selection::Model::SetStartIsActive(bool new_start_is_active) {
  if (start_is_active_ == new_start_is_active)
    return;
  start_is_active_ = new_start_is_active;
  NotifyChange();
}

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(const Range* range) : model_(new Model(*this, range)) {}

Selection::~Selection() {}

Offset Selection::anchor_offset() const {
  return model_->IsStartActive() ? model_->end() : model_->start();
}

Buffer* Selection::buffer() const {
  return model_->buffer();
}

Offset Selection::end() const {
  return model_->end();
}

Offset Selection::focus_offset() const {
  return model_->IsStartActive() ? model_->start() : model_->end();
}

Range* Selection::range() const {
  return model_.get();
}

Offset Selection::start() const {
  return model_->start();
}

void Selection::AddObserver(SelectionChangeObserver* observer) {
  model_->AddObserver(observer);
}

bool Selection::IsStartActive() const {
  return model_->IsStartActive();
}

void Selection::RemoveObserver(SelectionChangeObserver* observer) {
  model_->RemoveObserver(observer);
}

void Selection::SetStartIsActive(bool new_start_is_active) {
  model_->SetStartIsActive(new_start_is_active);
}

}  // namespace text
