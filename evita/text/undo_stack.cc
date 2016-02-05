// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/undo_stack.h"

#include <memory>
#include <ostream>

#include "base/auto_reset.h"
#include "base/logging.h"
#include "common/adopters/reverse.h"
#include "evita/text/buffer.h"
#include "evita/text/undo_step.h"
#include "evita/text/static_range.h"

std::ostream& operator<<(std::ostream& ostream, text::UndoStack::State state) {
  return ostream << static_cast<int>(state);
}

namespace text {

//////////////////////////////////////////////////////////////////////
//
// UndoStack
//
UndoStack::UndoStack(Buffer* pBuffer)
    : buffer_(pBuffer), state_(State::Normal) {
  buffer_->AddObserver(this);
}

UndoStack::~UndoStack() {
  Clear();
}

bool UndoStack::CanRedo() const {
  return !redo_steps_.empty();
}

bool UndoStack::CanUndo() const {
  return !undo_steps_.empty();
}

void UndoStack::Clear() {
  redo_steps_.clear();
  undo_steps_.clear();
}

void UndoStack::BeginUndoGroup(const base::string16& name) {
  DCHECK_EQ(State::Normal, state_);
  auto begin_step = std::make_unique<BeginUndoStep>(name);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, begin_step.get()))
    return undo_steps_.pop_back();
  undo_steps_.push_back(std::move(begin_step));
}

void UndoStack::EndUndoGroup(const base::string16& name) {
  DCHECK_EQ(State::Normal, state_);
  auto end_step = std::make_unique<EndUndoStep>(name);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, end_step.get()))
    return undo_steps_.pop_back();
  undo_steps_.push_back(std::move(end_step));
}

Offset UndoStack::Redo(Offset offset, int count) {
  if (redo_steps_.empty())
    return Offset::Invalid();

  for (const auto& step : common::adopters::reverse(redo_steps_)) {
    DCHECK(!step->is<EndUndoStep>());
    if (step->is<BeginUndoStep>())
      continue;
    if (step->GetBeforeRedo() != offset)
      return step->GetBeforeRedo();
    break;
  }

  base::AutoReset<State> state_scope(&state_, State::Redo);

  auto depth = 0;
  auto result_offset = offset;
  while (count > 0) {
    DCHECK(!redo_steps_.empty());
    undo_steps_.push_back(std::move(redo_steps_.back()));
    redo_steps_.pop_back();
    const auto step = undo_steps_.back().get();
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

Offset UndoStack::Undo(Offset offset, int count) {
  if (undo_steps_.empty())
    return Offset::Invalid();

  base::AutoReset<State> state_scope(&state_, State::Undo);

  for (const auto& step : common::adopters::reverse(undo_steps_)) {
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
    redo_steps_.push_back(std::move(undo_steps_.back()));
    undo_steps_.pop_back();
    const auto step = redo_steps_.back().get();
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
void UndoStack::DidInsertBefore(const StaticRange& range) {
  const auto start = range.start();
  const auto end = range.end();
  const auto length = range.length();

  if (state_ == State::Redo) {
    DCHECK(undo_steps_.back()->is<InsertUndoStep>());
    return;
  }

  if (state_ == State::Undo) {
    DCHECK(redo_steps_.back()->is<DeleteUndoStep>());
    return;
  }

  auto insert_step =
      std::make_unique<InsertUndoStep>(buffer_->revision() - 1, start, end);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, insert_step.get()))
    return;
  undo_steps_.push_back(std::move(insert_step));
}

void UndoStack::WillDeleteAt(const StaticRange& range) {
  const auto start = range.start();
  const auto end = range.end();
  const auto length = range.length();
  const auto& text = buffer_->GetText(start, end);
  if (state_ == State::Redo) {
    DCHECK(undo_steps_.back()->is<DeleteUndoStep>());
    return;
  }

  if (state_ == State::Undo) {
    const auto& insert_step = redo_steps_.back()->as<InsertUndoStep>();
    insert_step->set_text(text);
    return;
  }

  auto delete_step =
      std::make_unique<DeleteUndoStep>(buffer_->revision(), start, end, text);
  if (CanUndo() && undo_steps_.back()->TryMerge(buffer_, delete_step.get()))
    return;
  undo_steps_.push_back(std::move(delete_step));
}

}  // namespace text
