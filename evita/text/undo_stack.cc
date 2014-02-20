// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/undo_stack.h"

#include <memory>
#include <ostream>

#include "base/logging.h"
#include "common/adoptors/reverse.h"
#include "common/memory/scoped_change.h"
#include "evita/text/buffer.h"
#include "evita/text/undo_step.h"

std::ostream& operator<<(std::ostream& ostream, text::UndoStack::State state) {
  return ostream << static_cast<int>(state);
}

namespace text {

UndoStack::UndoStack(Buffer* pBuffer)
    : buffer_(pBuffer), state_(State::Normal) {
  buffer_->AddObserver(this);
}

UndoStack::~UndoStack() {
  Empty();
}

bool UndoStack::CanRedo() const {
  return !redo_steps_.empty();
}

bool UndoStack::CanUndo() const {
  return !undo_steps_.empty();
}

void UndoStack::Empty() {
  for (auto undo_step : undo_steps_) {
    delete undo_step;
  }
  undo_steps_.clear();
  for (auto redo_step : redo_steps_) {
    delete redo_step;
  }
  redo_steps_.clear();
}

void UndoStack::BeginUndoGroup(const base::string16& name) {
  DCHECK_EQ(State::Normal, state_);
  auto begin_step = std::make_unique<BeginUndoStep>(name);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, begin_step.get())) {
    delete undo_steps_.back();
    undo_steps_.pop_back();
    return;
  }
  undo_steps_.push_back(begin_step.release());
}

void UndoStack::EndUndoGroup(const base::string16& name) {
  DCHECK_EQ(State::Normal, state_);
  auto end_step = std::make_unique<EndUndoStep>(name);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, end_step.get())) {
    delete undo_steps_.back();
    undo_steps_.pop_back();
    return;
  }
  undo_steps_.push_back(end_step.release());
}

Posn UndoStack::Redo(Posn offset, Count count) {
  if (redo_steps_.empty())
    return -1;

  for (const auto step : common::adoptors::reverse(redo_steps_)) {
    DCHECK(!step->is<EndUndoStep>());
    if (step->is<BeginUndoStep>())
      continue;
    if (step->GetBeforeRedo() != offset)
      return step->GetBeforeRedo();
    break;
  }

  common::ScopedChange<State> state_scope(state_, State::Redo);

  auto depth = 0;
  auto result_offset = offset;
  while (count > 0) {
    DCHECK(!redo_steps_.empty());
    auto const step = redo_steps_.back();
    redo_steps_.pop_back();
    undo_steps_.push_back(step);
    if (step->is<BeginUndoStep>()) {
      ++depth;
    } else if (step->is<EndUndoStep>()) {
      DCHECK(depth);
      --depth;
    } else {
      step->Redo(buffer_);
      result_offset = step->GetAfterRedo();
    }
    if (!depth)
      --count;
  }
  DCHECK(!depth);
  return result_offset;
}

Posn UndoStack::Undo(Posn offset, Count count) {
  if (undo_steps_.empty())
    return -1;

  common::ScopedChange<State> state_scope(state_, State::Undo);

  for (const auto step : common::adoptors::reverse(undo_steps_)) {
    DCHECK(!step->is<BeginUndoStep>());
    if (step->is<EndUndoStep>())
      continue;
    if (step->GetBeforeUndo() != offset)
      return step->GetBeforeUndo();
    break;
  }

  auto depth = 0;
  auto result_offset = offset;
  while (count > 0) {
    DCHECK(!undo_steps_.empty());
    auto const step = undo_steps_.back();
    undo_steps_.pop_back();
    redo_steps_.push_back(step);
    if (step->is<EndUndoStep>()) {
      ++depth;
    } else if (step->is<BeginUndoStep>()) {
      DCHECK(depth);
      --depth;
    } else {
      step->Undo(buffer_);
      result_offset = step->GetAfterUndo();
    }

    if (!depth)
      --count;
  }
  DCHECK(!depth);
  return result_offset;
}

// BufferMutationObserver
void UndoStack::DidInsertAt(Posn start, size_t length) {
  auto const end = static_cast<Posn>(start + length);

  if (state_ == State::Redo) {
    DCHECK(undo_steps_.back()->is<InsertUndoStep>());
    return;
  }

  if (state_ == State::Undo) {
    DCHECK(redo_steps_.back()->is<DeleteUndoStep>());
    return;
  }

  auto insert_step = std::make_unique<InsertUndoStep>(start, end);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, insert_step.get())) {
    buffer_->IncCharTick(-1);
    return;
  }
  undo_steps_.push_back(insert_step.release());
}

void UndoStack::WillDeleteAt(Posn start, size_t length) {
  auto const end = static_cast<Posn>(start + length);
  auto text = buffer_->GetText(start, end);
  if (state_ == State::Redo) {
    DCHECK(undo_steps_.back()->is<DeleteUndoStep>());
    return;
  }

  if (state_ == State::Undo) {
    auto const insert_step = redo_steps_.back()->as<InsertUndoStep>();
    insert_step->set_text(text);
    return;
  }

  auto delete_step = std::make_unique<DeleteUndoStep>(start, end, text);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, delete_step.get())) {
    buffer_->IncCharTick(-1);
    return;
  }
  undo_steps_.push_back(delete_step.release());
}

}  // namespace text

