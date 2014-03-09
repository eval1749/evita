// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/undo_step.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/undo_stack.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// UndoStep
//
UndoStep::UndoStep() {
}

UndoStep::~UndoStep() {
}

Posn UndoStep::GetAfterRedo() const {
  return -1;
}

Posn UndoStep::GetAfterUndo() const {
  return -1;
}

Posn UndoStep::GetBeforeRedo() const {
  return -1;
}

Posn UndoStep::GetBeforeUndo() const {
  return -1;
}

void UndoStep::Redo(Buffer*) {
}

void UndoStep::Undo(Buffer*) {
}

//////////////////////////////////////////////////////////////////////
//
// NamedUndoStep
//
NamedUndoStep::NamedUndoStep(const base::string16& name) : name_(name) {
  DCHECK(!name_.empty());
}

NamedUndoStep::~NamedUndoStep() {
}

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
TextUndoStep::TextUndoStep(Posn start, Posn end)
    : end_(end), start_(start) {
  DCHECK_LE(start_, end_);
}

TextUndoStep::~TextUndoStep() {
}

void TextUndoStep::set_text(const base::string16& text) {
  DCHECK(!text.empty());
  text_ = text;
}

void TextUndoStep::set_text(base::string16&& text) {
  DCHECK(!text.empty());
  text_ = std::move(text);
}

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//

BeginUndoStep::BeginUndoStep(const base::string16& name)
    : NamedUndoStep(name) {
}

BeginUndoStep::~BeginUndoStep() {
}

// UndoStep
bool BeginUndoStep::TryMerge(const Buffer*, const UndoStep* new_undo_step) {
  if (auto const end = new_undo_step->as<EndUndoStep>()) {
    // Remove empty undo group.
    DCHECK_EQ(name(), end->name());
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// DeleteUndoStep
//
DeleteUndoStep::DeleteUndoStep(Posn start, Posn end, const base::string16& text)
    : TextUndoStep(start, end) {
  set_text(text);
}

DeleteUndoStep::~DeleteUndoStep() {
}

// UndoStep
Posn DeleteUndoStep::GetAfterRedo() const {
  return start();
}

Posn DeleteUndoStep::GetAfterUndo() const {
  return end();
}

Posn DeleteUndoStep::GetBeforeRedo() const {
  return end();
}

Posn DeleteUndoStep::GetBeforeUndo() const {
  return start();
}

void DeleteUndoStep::Redo(Buffer* buffer) {
  buffer->Delete(start(), end());
}

// Merges new delete information into the last delete UndoStep if
// o New delete doesn't start with newline.
// o New delete doesn't end with newline.
bool DeleteUndoStep::TryMerge(const Buffer*, const UndoStep* other) {
  auto const delete_step = other->as<DeleteUndoStep>();
  if (!delete_step)
    return false;

  if (delete_step->text().front() == 0x0A ||
      delete_step->text().back() == 0x0A) {
    return false;
  }

  // For [Backspace] key
  // 1. abc|
  // 2. ab|
  if (start() == delete_step->end()) {
    set_text(std::move(delete_step->text() + text()));
    set_start(delete_step->start());
    return true;
  }

  // For [Delete] key
  // 1. a|bc
  // 2. a|c
  if (start() == delete_step->start()) {
    set_text(std::move(text() + delete_step->text()));
    set_end(end() + delete_step->end() - delete_step->start());
    return true;
  }

  return false;
}

void DeleteUndoStep::Undo(Buffer* buffer) {
  buffer->Insert(start(), text().data(), static_cast<Count>(text().length()));
  // -1 for |Buffer::Insert()| in this function.
  // -1 for |Buffer::Delete()| which creates this |DeleteUndoStep|.
  buffer->IncCharTick(-2);
}

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
EndUndoStep::EndUndoStep(const base::string16& name)
    : NamedUndoStep(name) {
}

EndUndoStep::~EndUndoStep() {
}

// UndoStep
bool EndUndoStep::TryMerge(const Buffer*, const UndoStep*) {
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
InsertUndoStep::InsertUndoStep(Posn start, Posn end)
    : TextUndoStep(start, end) {
}

InsertUndoStep::~InsertUndoStep() {
}

// Merge "Insert" UndoStep if
// o [last][new]
// o [new][last]
// and there is no newline between last and new.
bool InsertUndoStep::TryMerge(const Buffer* buffer, const UndoStep* other) {
  auto const insert_step = other->as<InsertUndoStep>();
  if (!insert_step)
    return false;

  if (end() == insert_step->start()) {
    // [last][new]
    if (buffer->GetCharAt(end() - 1) != 0x0A) {
      set_end(insert_step->end());
      return true;
    }
  } else if (start() == insert_step->end()) {
    // [new][last]
    if (buffer->GetCharAt(start() - 1) != 0x0A) {
      set_start(insert_step->start());
      return true;
    }
  }
  return false;
}

// UndoStep
Posn InsertUndoStep::GetAfterRedo() const {
  return end();
}

Posn InsertUndoStep::GetAfterUndo() const {
  return start();
}

Posn InsertUndoStep::GetBeforeRedo() const {
  return start();
}

Posn InsertUndoStep::GetBeforeUndo() const {
  return end();
}

void InsertUndoStep::Redo(Buffer* buffer) {
  DCHECK_EQ(text().length(), static_cast<size_t>(end() - start()));
  buffer->Insert(start(), text().data(), static_cast<Count>(text().length()));
}

void InsertUndoStep::Undo(Buffer* buffer) {
  buffer->Delete(start(), end());
  // -1 for |Buffer::Delete()| in this function.
  // -1 for |Buffer::Insert()| which creates this |InsertUndoStep|.
  buffer->IncCharTick(-2);
}

}  // namespace text
