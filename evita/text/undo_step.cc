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
UndoStep::UndoStep() {}

UndoStep::~UndoStep() {}

Offset UndoStep::GetAfterRedo() const {
  return Offset::Invalid();
}

Offset UndoStep::GetAfterUndo() const {
  return Offset::Invalid();
}

Offset UndoStep::GetBeforeRedo() const {
  return Offset::Invalid();
}

Offset UndoStep::GetBeforeUndo() const {
  return Offset::Invalid();
}

void UndoStep::Redo(Buffer* buffer) {
  DCHECK(buffer);
}

void UndoStep::Undo(Buffer* buffer) {
  DCHECK(buffer);
}

//////////////////////////////////////////////////////////////////////
//
// NamedUndoStep
//
NamedUndoStep::NamedUndoStep(const base::string16& name) : name_(name) {
  DCHECK(!name_.empty());
}

NamedUndoStep::~NamedUndoStep() {}

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
TextUndoStep::TextUndoStep(Offset start, Offset end)
    : end_(end), start_(start) {
  DCHECK_LE(start_, end_);
}

TextUndoStep::~TextUndoStep() {}

void TextUndoStep::set_text(const base::string16& text) {
  DCHECK(!text.empty());
  text_ = text;
}

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//

BeginUndoStep::BeginUndoStep(const base::string16& name)
    : NamedUndoStep(name) {}

BeginUndoStep::~BeginUndoStep() {}

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
DeleteUndoStep::DeleteUndoStep(Offset start,
                               Offset end,
                               const base::string16& text)
    : TextUndoStep(start, end) {
  set_text(text);
}

DeleteUndoStep::~DeleteUndoStep() {}

// UndoStep
Offset DeleteUndoStep::GetAfterRedo() const {
  return start();
}

Offset DeleteUndoStep::GetAfterUndo() const {
  return end();
}

Offset DeleteUndoStep::GetBeforeRedo() const {
  return end();
}

Offset DeleteUndoStep::GetBeforeUndo() const {
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
    set_end(end() + (delete_step->end() - delete_step->start()));
    return true;
  }

  return false;
}

void DeleteUndoStep::Undo(Buffer* buffer) {
  buffer->InsertBefore(start(), text());
  // -1 for |Buffer::Insert()| in this function.
  // -1 for |Buffer::Delete()| which creates this |DeleteUndoStep|.
  buffer->IncCharTick(-2);
}

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
EndUndoStep::EndUndoStep(const base::string16& name) : NamedUndoStep(name) {}

EndUndoStep::~EndUndoStep() {}

// UndoStep
bool EndUndoStep::TryMerge(const Buffer*, const UndoStep*) {
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
InsertUndoStep::InsertUndoStep(Offset start, Offset end)
    : TextUndoStep(start, end) {}

InsertUndoStep::~InsertUndoStep() {}

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
    if (buffer->GetCharAt(end() - OffsetDelta(1)) != 0x0A) {
      set_end(insert_step->end());
      return true;
    }
  } else if (start() == insert_step->end()) {
    // [new][last]
    if (buffer->GetCharAt(start() - OffsetDelta(1)) != 0x0A) {
      set_start(insert_step->start());
      return true;
    }
  }
  return false;
}

// UndoStep
Offset InsertUndoStep::GetAfterRedo() const {
  return end();
}

Offset InsertUndoStep::GetAfterUndo() const {
  return start();
}

Offset InsertUndoStep::GetBeforeRedo() const {
  return start();
}

Offset InsertUndoStep::GetBeforeUndo() const {
  return end();
}

void InsertUndoStep::Redo(Buffer* buffer) {
  DCHECK_EQ(text().length(), static_cast<size_t>((end() - start()).value()));
  buffer->InsertBefore(start(), text());
}

void InsertUndoStep::Undo(Buffer* buffer) {
  buffer->Delete(start(), end());
  // -1 for |Buffer::Delete()| in this function.
  // -1 for |Buffer::Insert()| which creates this |InsertUndoStep|.
  buffer->IncCharTick(-2);
}

}  // namespace text
