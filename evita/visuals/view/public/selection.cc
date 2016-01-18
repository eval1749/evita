// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/public/selection.h"

#include "base/logging.h"
#include "base/timer/timer.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/media_state.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/view/public/selection_model.h"
#include "evita/visuals/view/public/selection_observer.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(ViewLifecycle* lifecycle)
    : ViewLifecycleClient(lifecycle),
      caret_timer_(new base::RepeatingTimer()),
      model_(new SelectionModel()) {
  document().AddObserver(this);
}

Selection::~Selection() {
  document().RemoveObserver(this);
}

const Node& Selection::anchor_node() const {
  return model_->anchor_node();
}

int Selection::anchor_offset() const {
  return model_->anchor_offset();
}

const Node& Selection::focus_node() const {
  return model_->focus_node();
}

int Selection::focus_offset() const {
  return model_->focus_offset();
}

bool Selection::is_caret() const {
  return model_->is_caret();
}

bool Selection::is_none() const {
  return model_->is_none();
}

bool Selection::is_range() const {
  return model_->is_range();
}

void Selection::AddObserver(SelectionObserver* observer) const {
  observers_.AddObserver(observer);
}

void Selection::Clear() {
  DCHECK(lifecycle()->AllowsSelectionChanges()) << lifecycle();
  if (model_->is_none())
    return;
  const auto old_model = *model_;
  model_->Clear();
  caret_timer_->Stop();
  FOR_EACH_OBSERVER(SelectionObserver, observers_,
                    DidChangeSelection(old_model, *model_));
}

void Selection::Collapse(Node* node, int offset) {
  DCHECK(lifecycle()->AllowsSelectionChanges()) << lifecycle();
  SelectionModel new_model;
  new_model.Collapse(node, offset);
  if (*model_ == new_model)
    return;
  const auto old_model = *model_;
  *model_ = new_model;
  is_caret_on_ = true;
  caret_timer_->Stop();
  FOR_EACH_OBSERVER(SelectionObserver, observers_,
                    DidChangeSelection(old_model, *model_));
}

void Selection::DidFireCaretTimer() {
  is_caret_on_ = !is_caret_on_;
  FOR_EACH_OBSERVER(SelectionObserver, observers_, DidChangeCaretBlink());
}

void Selection::DidPaint() {
  if (is_none())
    return;
  if (caret_timer_->IsRunning())
    return;
  caret_timer_->Start(
      FROM_HERE, base::TimeDelta::FromMilliseconds(500),
      base::Bind(&Selection::DidFireCaretTimer, base::Unretained(this)));
}

void Selection::ExtendTo(Node* node, int offset) {
  DCHECK(lifecycle()->AllowsSelectionChanges()) << lifecycle();
  DCHECK(!is_none());
  SelectionModel new_model(*model_);
  new_model.ExtendTo(node, offset);
  if (*model_ == new_model)
    return;
  const auto old_model = *model_;
  *model_ = new_model;
  is_caret_on_ = true;
  caret_timer_->Stop();
  FOR_EACH_OBSERVER(SelectionObserver, observers_,
                    DidChangeSelection(old_model, *model_));
}

void Selection::RemoveObserver(SelectionObserver* observer) const {
  observers_.RemoveObserver(observer);
}

// css::MediaObserver
void Selection::DidChangeMediaState() {
  if (media().media_state() != css::MediaState::Inactive)
    return;
  caret_timer_->Stop();
}

// DocumentObserver
void Selection::WillRemoveChild(const ContainerNode& parent,
                                const Node& child) {
  model_->WillRemoveChild(parent, child);
}

}  // namespace visuals
