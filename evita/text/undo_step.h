// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_UNDO_STEP_H_
#define EVITA_TEXT_UNDO_STEP_H_

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/castable.h"
#include "evita/precomp.h"

namespace text {

class UndoStack;

//////////////////////////////////////////////////////////////////////
//
// UndoStep
//
class UndoStep : public common::Castable {
  DECLARE_CASTABLE_CLASS(UndoStep, Castable);

 public:
  virtual ~UndoStep();

  virtual Posn GetAfterRedo() const;
  virtual Posn GetAfterUndo() const;
  virtual Posn GetBeforeRedo() const;
  virtual Posn GetBeforeUndo() const;
  virtual void Redo(Buffer* buffer);
  virtual bool TryMerge(const Buffer* buffer, const UndoStep* other) = 0;
  virtual void Undo(Buffer* buffer);

 protected:
  UndoStep();

 private:
  DISALLOW_COPY_AND_ASSIGN(UndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// NamedUndoStep
//
class NamedUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(NamedUndoStep, UndoStep);

 public:
  const base::string16& name() const { return name_; }

 protected:
  explicit NamedUndoStep(const base::string16& name);
  ~NamedUndoStep() override;

 private:
  const base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(NamedUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
class TextUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(TextUndoStep, UndoStep);

 public:
  Posn end() const { return end_; }
  void set_end(Posn end) { end_ = end; }
  Posn start() const { return start_; }
  void set_start(Posn start) { start_ = start; }
  const base::string16& text() const { return text_; }
  void set_text(const base::string16& text);

 protected:
  TextUndoStep(Posn start, Posn end);
  ~TextUndoStep() override;

 private:
  Posn end_;
  Posn start_;
  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(TextUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//
class BeginUndoStep final : public NamedUndoStep {
  DECLARE_CASTABLE_CLASS(BeginUndoStep, NamedUndoStep);

 public:
  explicit BeginUndoStep(const base::string16& name);
  ~BeginUndoStep() final;

 private:
  // UndoStep
  bool TryMerge(const Buffer* buffer, const UndoStep* other) final;

  DISALLOW_COPY_AND_ASSIGN(BeginUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// DeleteUndoStep
//
class DeleteUndoStep final : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(DeleteUndoStep, TextUndoStep);

 public:
  DeleteUndoStep(Posn start, Posn end, const base::string16& text);
  ~DeleteUndoStep() final;

 private:
  // Recrod
  Posn GetAfterRedo() const final;
  Posn GetAfterUndo() const final;
  Posn GetBeforeRedo() const final;
  Posn GetBeforeUndo() const final;
  void Redo(Buffer* buffer) final;
  bool TryMerge(const Buffer* buffer, const UndoStep* other) final;
  void Undo(Buffer* buffer) final;

  DISALLOW_COPY_AND_ASSIGN(DeleteUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
class EndUndoStep final : public NamedUndoStep {
  DECLARE_CASTABLE_CLASS(EndUndoStep, NamedUndoStep);

 public:
  explicit EndUndoStep(const base::string16& name);
  ~EndUndoStep() final;

 private:
  // UndoStep
  bool TryMerge(const Buffer* buffer, const UndoStep* other) final;

  DISALLOW_COPY_AND_ASSIGN(EndUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
class InsertUndoStep final : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(InsertUndoStep, TextUndoStep);

 public:
  InsertUndoStep(Posn lStart, Posn lEnd);
  ~InsertUndoStep() final;

 private:
  // UndoStep
  Posn GetAfterRedo() const final;
  Posn GetAfterUndo() const final;
  Posn GetBeforeRedo() const final;
  Posn GetBeforeUndo() const final;
  void Redo(Buffer* buffer) final;

  bool TryMerge(const Buffer* buffer, const UndoStep* other) final;
  void Undo(Buffer* buffer) final;

  DISALLOW_COPY_AND_ASSIGN(InsertUndoStep);
};

}  // namespace text

#endif  // EVITA_TEXT_UNDO_STEP_H_
