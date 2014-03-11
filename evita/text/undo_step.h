// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_step_h)
#define INCLUDE_evita_text_undo_step_h

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/castable.h"

namespace text {

class UndoStack;

//////////////////////////////////////////////////////////////////////
//
// UndoStep
//
class UndoStep : public common::Castable {
  DECLARE_CASTABLE_CLASS(UndoStep, Castable);

  protected: UndoStep();
  public: virtual ~UndoStep();

  public: virtual Posn GetAfterRedo() const;
  public: virtual Posn GetAfterUndo() const;
  public: virtual Posn GetBeforeRedo() const;
  public: virtual Posn GetBeforeUndo() const;
  public: virtual void Redo(Buffer* buffer);
  public: virtual bool TryMerge(const Buffer* buffer,
                                const UndoStep* other) = 0;
  public: virtual void Undo(Buffer* buffer);

  DISALLOW_COPY_AND_ASSIGN(UndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// NamedUndoStep
//
class NamedUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(NamedUndoStep, UndoStep);

  private: const base::string16 name_;

  protected: NamedUndoStep(const base::string16& name);
  public: virtual ~NamedUndoStep();

  public: const base::string16& name() const { return name_; }

  DISALLOW_COPY_AND_ASSIGN(NamedUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
class TextUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(TextUndoStep, UndoStep);

  private: Posn end_;
  private: Posn start_;
  private: base::string16 text_;

  protected: TextUndoStep(Posn start, Posn end);
  public: virtual ~TextUndoStep();

  public: Posn end() const { return end_; }
  public: void set_end(Posn end) { end_ = end; }
  public: Posn start() const { return start_; }
  public: void set_start(Posn start) { start_ = start; }
  public: const base::string16& text() const { return text_; }
  public: void set_text(const base::string16& text);
  public: void set_text(base::string16&& text);

  DISALLOW_COPY_AND_ASSIGN(TextUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//
class BeginUndoStep : public NamedUndoStep {
  DECLARE_CASTABLE_CLASS(BeginUndoStep, NamedUndoStep);

  public: BeginUndoStep(const base::string16& name);
  public: virtual ~BeginUndoStep();

  // UndoStep
  private: virtual bool TryMerge(const Buffer* buffer,
                                 const UndoStep* other) override;

  DISALLOW_COPY_AND_ASSIGN(BeginUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// DeleteUndoStep
//
class DeleteUndoStep : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(DeleteUndoStep, TextUndoStep);

  public: DeleteUndoStep(Posn start, Posn end, const base::string16& text);
  public: virtual ~DeleteUndoStep();

  // Recrod
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* buffer) override;
  private: virtual bool TryMerge(const Buffer* buffer,
                                 const UndoStep* other) override;
  private: virtual void Undo(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(DeleteUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
class EndUndoStep : public NamedUndoStep {
  DECLARE_CASTABLE_CLASS(EndUndoStep, NamedUndoStep);

  public: EndUndoStep(const base::string16& name);
  public: virtual ~EndUndoStep();

  // UndoStep
  private: virtual bool TryMerge(const Buffer* buffer,
                                 const UndoStep* other) override;

  DISALLOW_COPY_AND_ASSIGN(EndUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
class InsertUndoStep : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(InsertUndoStep, TextUndoStep);

  public: InsertUndoStep(Posn lStart, Posn lEnd);
  public: virtual ~InsertUndoStep();

  // UndoStep
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* buffer) override;;
  private: virtual bool TryMerge(const Buffer* buffer,
                                 const UndoStep* other) override;
  private: virtual void Undo(Buffer* buffer) override;;

  DISALLOW_COPY_AND_ASSIGN(InsertUndoStep);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_undo_step_h)
