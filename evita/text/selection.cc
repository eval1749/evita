// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/selection.h"

#include "base/observer_list.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/text/selection_change_observer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Model
//
class Selection::Model : public Range {
  private: base::ObserverList<SelectionChangeObserver> observers_;
  private: bool start_is_active_;

  public: Model(const Range* range);
  public: virtual ~Model();

  public: void AddObserver(SelectionChangeObserver* observer);
  private: void DidChangeRange() override;
  public: bool IsStartActive() const { return start_is_active_; }
  private: void NotifyChange();
  public: void RemoveObserver(SelectionChangeObserver* observer);
  public: void SetStartIsActive(bool new_start_is_active);

  DISALLOW_COPY_AND_ASSIGN(Model);
};

Selection::Model::Model(const Range* range)
    : Range(range->buffer(), range->start(), range->end()),
      start_is_active_(false) {
}

Selection::Model::~Model() {
}

void Selection::Model::AddObserver(SelectionChangeObserver* observer) {
  observers_.AddObserver(observer);
}

void Selection::Model::DidChangeRange() {
  NotifyChange();
}

void Selection::Model::NotifyChange() {
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
Selection::Selection(const Range* range) : model_(new Model(range)) {
}

Selection::~Selection() {
}

Posn Selection::anchor_offset() const {
  return model_->IsStartActive() ? model_->end() : model_->start();
}

Buffer* Selection::buffer() const {
  return model_->buffer();
}

Posn Selection::end() const {
  return model_->end();
}

Posn Selection::focus_offset() const {
  return model_->IsStartActive() ? model_->start() : model_->end();
}

Range* Selection::range() const {
  return model_.get();
}

Posn Selection::start() const {
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
